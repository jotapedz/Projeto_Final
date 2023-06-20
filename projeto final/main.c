#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <dirent.h>
#include <sys/time.h>

#ifdef _WIN32
#include <windows.h>
#define mkdir(path, mode) _mkdir(path)
#else
#include <sys/stat.h>
#endif

// Estrutura para armazenar uma imagem PGM
typedef struct {
    int width;
    int height;
    int maxValue;
    unsigned char** pixels;
} PGMImage;

// Estrutura para armazenar as coordenadas de uma sub-imagem
typedef struct {
    char name[100];
    int x;
    int y;
} SubImageCoordinates;

// Função para ler uma imagem PGM binária
PGMImage* readPGMBinary(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        printf("Erro ao abrir o arquivo: %s\n", filename);
        return NULL;
    }

    // Lendo o cabeçalho
    char format[3];
    fscanf(file, "%2s", format);
    if (strcmp(format, "P5") != 0) {
        printf("Formato inválido! Deve ser P5 (PGM binário)\n");
        fclose(file);
        return NULL;
    }

    int width, height, maxValue;
    fscanf(file, "%d %d %d", &width, &height, &maxValue);
    fgetc(file); // Lendo o caractere de nova linha

    // Alocando memória para os pixels
    unsigned char** pixels = (unsigned char**)malloc(height * sizeof(unsigned char*));
    for (int i = 0; i < height; i++) {
        pixels[i] = (unsigned char*)malloc(width * sizeof(unsigned char));
    }

    // Lendo os pixels
    for (int i = 0; i < height; i++) {
        fread(pixels[i], sizeof(unsigned char), width, file);
    }

    fclose(file);

    // Criando a estrutura de imagem
    PGMImage* image = (PGMImage*)malloc(sizeof(PGMImage));
    image->width = width;
    image->height = height;
    image->maxValue = maxValue;
    image->pixels = pixels;

    return image;
}

// Função para salvar uma imagem PGM binária
void savePGMBinary(const char* filename, const PGMImage* image) {
    FILE* file = fopen(filename, "wb");
    if (file == NULL) {
        printf("Erro ao abrir o arquivo: %s\n", filename);
        return;
    }

    // Escrevendo o cabeçalho
    fprintf(file, "P5\n");
    fprintf(file, "%d %d\n", image->width, image->height);
    fprintf(file, "%d\n", image->maxValue);

    // Escrevendo os pixels
    for (int i = 0; i < image->height; i++) {
        fwrite(image->pixels[i], sizeof(unsigned char), image->width, file);
    }

    fclose(file);
}

// Função para liberar a memória de uma imagem PGM
void freePGMImage(PGMImage* image) {
   for (int i = 0; i < image->height; i++) {
        free(image->pixels[i]);
    }
    free(image->pixels);
    free(image);
}

// Função para calcular o erro médio quadrático (MSE) entre duas imagens
double calculateMSE(const PGMImage* image1, const PGMImage* image2, int startX, int startY) {
     int width = image2->width;
    int height = image2->height;

    double sum = 0.0;
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int pixel1 = image1->pixels[startY + i][startX + j];
            int pixel2 = image2->pixels[i][j];
            sum += pow(pixel1 - pixel2, 2);
        }
    }
    double mse = sum / (width * height);
    return mse;
}

// Função para aplicar o filtro da média em uma imagem PGM
void applyFilter(const unsigned char** input, unsigned char** output, int width, int height) {
        for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int sum = 0;
            for (int k = i - 1; k <= i + 1; k++) {
                for (int l = j - 1; l <= j + 1; l++) {
                    if (k >= 0 && k < height && l >= 0 && l < width) {
                        sum += input[k][l];
                    }
                }
            }
            output[i][j] = sum / 9;
        }
    }
}

// Função principal
int main() {
    char inputFilename[100];
    printf("Digite o nome do arquivo de entrada: ");
    scanf("%s", inputFilename);

    PGMImage* inputImage = readPGMBinary(inputFilename);
    if (inputImage == NULL) {
        return 1;
    }

    int numSubImages;
    printf("Digite a quantidade de sub-imagens: ");
    scanf("%d", &numSubImages);

    int subImageWidth, subImageHeight;
    printf("Digite as dimensões das sub-imagens (largura altura): ");
    scanf("%d %d", &subImageWidth, &subImageHeight);

    char outputDirectory[100];
    printf("Digite o diretório de saída: ");
    scanf("%s", outputDirectory);

    // Criando o diretório de saída se não existir
    #ifdef _WIN32
    #include <direct.h>
    #define MKDIR(path) _mkdir(path)
    #else
    #include <sys/stat.h>
    #define MKDIR(path) mkdir(path, 0777)
    #endif

    // Gerando e salvando as sub-imagens
    for (int i = 0; i < numSubImages; i++) {
        // Gerando um ponto pseudo-aleatório dentro dos limites da imagem
        int startX = rand() % (inputImage->width - subImageWidth + 1);
        int startY = rand() % (inputImage->height - subImageHeight + 1);

        // Criando a sub-imagem
        PGMImage* subImage = (PGMImage*)malloc(sizeof(PGMImage));
        subImage->width = subImageWidth;
        subImage->height = subImageHeight;
        subImage->maxValue = inputImage->maxValue;
        subImage->pixels = (unsigned char**)malloc(subImageHeight * sizeof(unsigned char*));
        for (int j = 0; j < subImageHeight; j++) {
            subImage->pixels[j] = (unsigned char*)malloc(subImageWidth * sizeof(unsigned char));
            memcpy(subImage->pixels[j], &inputImage->pixels[startY + j][startX], subImageWidth);
        }

        // Aplicando o filtro da média
        unsigned char** filteredPixels = (unsigned char**)malloc(subImageHeight * sizeof(unsigned char*));
        for (int j = 0; j < subImageHeight; j++) {
            filteredPixels[j] = (unsigned char*)malloc(subImageWidth * sizeof(unsigned char));
        }
        applyFilter((const unsigned char**)subImage->pixels, filteredPixels, subImageWidth, subImageHeight);

        // Criando o nome do arquivo de saída
        char outputFilename[200];
        sprintf(outputFilename, "%s/subimage_%d.pgm", outputDirectory, i + 1);

        // Salvando a sub-imagem filtrada
        savePGMBinary(outputFilename, subImage);

        // Liberando a memória da sub-imagem e dos pixels filtrados
        freePGMImage(subImage);
        for (int j = 0; j < subImageHeight; j++) {
            free(filteredPixels[j]);
        }
        free(filteredPixels);
    }

    // Liberando a memória da imagem de entrada
    freePGMImage(inputImage);

    char subImagesDirectory[100];
    printf("Digite o diretório das sub-imagens: ");
    scanf("%s", subImagesDirectory);

    DIR* directory = opendir(subImagesDirectory);
    if (directory == NULL) {
        printf("Erro ao abrir o diretório das sub-imagens\n");
        return 1;
    }

    // Lendo a imagem principal
    inputImage = readPGMBinary(inputFilename);
    if (inputImage == NULL) {
        closedir(directory);
        return 1;
    }

    struct timeval startTime, endTime;
    gettimeofday(&startTime, NULL);

    // Buscando e processando cada sub-imagem no diretório
    struct dirent* entry;
    while ((entry = readdir(directory)) != NULL) {
        if (entry->d_type == DT_REG) {
            char subImageFilename[200];
            sprintf(subImageFilename, "%s/%s", subImagesDirectory, entry->d_name);

            // Lendo a sub-imagem
            PGMImage* subImage = readPGMBinary(subImageFilename);
            if (subImage == NULL) {
                continue;
            }

            int subImageWidth = subImage->width;
            int subImageHeight = subImage->height;

            // Procurando a região correspondente na imagem principal com base no MSE
            int startX = 0;
            int startY = 0;
            double minMSE = calculateMSE(inputImage, subImage, startX, startY);
            for (int i = 0; i <= inputImage->height - subImageHeight; i++) {
                for (int j = 0; j <= inputImage->width - subImageWidth; j++) {
                    double mse = calculateMSE(inputImage, subImage, j, i);
                    if (mse < minMSE) {
                        minMSE = mse;
                        startX = j;
                        startY = i;
                    }
                }
            }

            // Salvando as coordenadas da sub-imagem
            SubImageCoordinates coordinates;
            strncpy(coordinates.name, entry->d_name, sizeof(coordinates.name));
            coordinates.x = startX;
            coordinates.y = startY;

            // Salvando as coordenadas no arquivo de texto
            FILE* outputFile = fopen("coordinates.txt", "a");
            if (outputFile == NULL) {
                printf("Erro ao abrir o arquivo de coordenadas\n");
                freePGMImage(subImage);
                continue;
            }

            fprintf(outputFile, "%s, %d, %d\n", coordinates.name, coordinates.x, coordinates.y);
            fclose(outputFile);

            freePGMImage(subImage);
        }
    }

    gettimeofday(&endTime, NULL);
    double elapsedTime = (endTime.tv_sec - startTime.tv_sec) + (endTime.tv_usec - startTime.tv_usec) / 1000000.0;
    printf("Tempo total para busca das sub-imagens: %.2f segundos\n", elapsedTime);

    closedir(directory);
    freePGMImage(inputImage);

    return 0;
}
