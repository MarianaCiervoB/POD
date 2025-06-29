#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 1024
#define MAX_ENTRIES 4000000 /* Para cobrir todos os inscritos */
typedef struct {
    char inscricao[32];
    char municipio[100];
    char uf[3];
    float nota_redacao;
} Inscricao;

Inscricao inscricoes[MAX_ENTRIES];
int total_inscricoes = 0;

/* Compara inscrições para busca binária e qsort */
int compara_inscricao(const void *a, const void *b) {
    return strcmp(((Inscricao *)a)->inscricao, ((Inscricao *)b)->inscricao);
}

void processar_linha(char *linha) {
    int coluna = 0;
    char *inicio = linha;
    char campo[256];
    int i = 0;

    char campo_inscricao[32] = "";
    char campo_municipio[100] = "";
    char campo_uf[4] = "";
    char campo_redacao[16] = "";

    while (*linha != '\0' && coluna <= 51) {
        if (*linha == ';' || *(linha + 1) == '\0') {
            int tamanho = linha - inicio;
            if (*(linha + 1) == '\0') tamanho++; /* último campo */

            if (tamanho >= sizeof(campo)) tamanho = sizeof(campo) - 1;

            strncpy(campo, inicio, tamanho);
            campo[tamanho] = '\0';

            /* Armazena somente os campos desejados */
            if (coluna == 0) {
                strncpy(campo_inscricao, campo, sizeof(campo_inscricao) - 1);
            } else if (coluna == 20) {
                strncpy(campo_municipio, campo, sizeof(campo_municipio) - 1);
            } else if (coluna == 22) {
                strncpy(campo_uf, campo, sizeof(campo_uf) - 1);
            } else if (coluna == 50) {
                strncpy(campo_redacao, campo, sizeof(campo_redacao) - 1);
            }

            /* Próxima coluna */
            coluna++;
            linha++;  /* pula o ';' */
            inicio = linha;
        } else {
            linha++;
        }
    }

    if (coluna < 51) {
        printf("Linha malformada (colunas insuficientes): %s\n", linha);
        return;
    }

    /* Preenche a struct */
    strncpy(inscricoes[total_inscricoes].inscricao, campo_inscricao, sizeof(inscricoes[0].inscricao) - 1);
    inscricoes[total_inscricoes].inscricao[sizeof(inscricoes[0].inscricao) - 1] = '\0';

    strncpy(inscricoes[total_inscricoes].municipio, campo_municipio, sizeof(inscricoes[0].municipio) - 1);
    inscricoes[total_inscricoes].municipio[sizeof(inscricoes[0].municipio) - 1] = '\0';

    strncpy(inscricoes[total_inscricoes].uf, campo_uf, sizeof(inscricoes[0].uf) - 1);
    inscricoes[total_inscricoes].uf[sizeof(inscricoes[0].uf) - 1] = '\0';

    campo_redacao[strcspn(campo_redacao, "\r\n")] = '\0'; /* limpar quebra de linha */
    inscricoes[total_inscricoes].nota_redacao = atof(campo_redacao);

    total_inscricoes++;
}

void carregar_dados(const char *arquivo) {
    FILE *fp = fopen(arquivo, "rb");
    char linha[MAX_LINE];

    if (!fp) {
        printf("Erro ao abrir o arquivo.\n");
        exit(1);
    }

    fgets(linha, MAX_LINE, fp); /* Pula cabeçalho */

    while (fgets(linha, MAX_LINE, fp) && total_inscricoes < MAX_ENTRIES) {
        linha[strcspn(linha, "\n")] = '\0';
        processar_linha(linha);
    }

    fclose(fp);

    if (total_inscricoes > 0) {
        printf("DEBUG: Primeira inscrição carregada: %s - %s/%s - %.1f\n",
            inscricoes[0].inscricao,
            inscricoes[0].municipio,
            inscricoes[0].uf,
            inscricoes[0].nota_redacao);
    }

    qsort(inscricoes, total_inscricoes, sizeof(Inscricao), compara_inscricao);
}

void buscar_por_inscricao(const char *num) {
    Inscricao chave;
    Inscricao *resultado;
    strncpy(chave.inscricao, num, sizeof(chave.inscricao) - 1);
    chave.inscricao[sizeof(chave.inscricao) - 1] = '\0';

    resultado = bsearch(&chave, inscricoes, total_inscricoes, sizeof(Inscricao), compara_inscricao);

    if (resultado) {
        printf("Inscrição encontrada:\n");
        printf("Número: %s\nMunicípio: %s\nUF: %s\nNota Redação: %.1f\n",
               resultado->inscricao, resultado->municipio, resultado->uf, resultado->nota_redacao);
    } else {
        printf("Inscrição %s não encontrada.\n", num);
    }
}

void listar_por_municipio_uf(const char *municipio, const char *uf) {
    int encontrou = 0;
    int i;

    for (i = 0; i < total_inscricoes; i++) {
        if (strcmp(inscricoes[i].municipio, municipio) == 0 &&
            strcmp(inscricoes[i].uf, uf) == 0) {
            printf("Inscrição: %s | Nota Redação: %.1f\n",
                   inscricoes[i].inscricao, inscricoes[i].nota_redacao);
            encontrou = 1;
        }
    }

    if (!encontrou) {
        printf("Nenhuma inscrição encontrada para %s/%s.\n", municipio, uf);
    }
}

void listar_maiores_redacoes() {
    float maior = 0.0;
    int i;

    for (i = 0; i < total_inscricoes; i++) {
        if (inscricoes[i].nota_redacao > maior) {
            maior = inscricoes[i].nota_redacao;
        }
    }

    printf("Maior nota de redação: %.1f\n", maior);
    printf("Inscrições com esta nota:\n");

    for (i = 0; i < total_inscricoes; i++) {
        if (inscricoes[i].nota_redacao == maior) {
            printf("Inscrição: %s | Município: %s | UF: %s\n",
                   inscricoes[i].inscricao, inscricoes[i].municipio, inscricoes[i].uf);
        }
    }
}

void menu() {
    int op;
    char municipio[100];
    char uf[3];
    char num[32];

    do {
        printf("\n===== MENU ENEM 2023 =====\n");
        printf("1. Buscar por número de inscrição\n");
        printf("2. Listar por município + UF\n");
        printf("3. Listar maiores notas de redação\n");
        printf("0. Sair\n");
        printf("Escolha: ");
        scanf("%d", &op);
        getchar();

        switch (op) {
            case 1:
                printf("Digite o número da inscrição: ");
                fgets(num, sizeof(num), stdin);
                num[strcspn(num, "\n")] = '\0';
                buscar_por_inscricao(num);
                break;
            case 2:
                printf("Digite o município: ");
                fgets(municipio, sizeof(municipio), stdin);
                municipio[strcspn(municipio, "\n")] = '\0';

                printf("Digite a UF: ");
                fgets(uf, sizeof(uf), stdin);
                uf[strcspn(uf, "\n")] = '\0';

                listar_por_municipio_uf(municipio, uf);
                break;
            case 3:
                listar_maiores_redacoes();
                break;
            case 0:
                printf("Saindo...\n");
                break;
            default:
                printf("Opção inválida.\n");
        }
    } while (op != 0);
}

int main() {
    setlocale(LC_NUMERIC, "C");
    printf("Carregando dados...\n");
    carregar_dados("MICRODADOS_ENEM_2023.csv");
    printf("Dados carregados. Total de inscrições: %d\n", total_inscricoes);
    menu();
    return 0;
}
