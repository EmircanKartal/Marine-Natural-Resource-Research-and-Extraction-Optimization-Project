#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <stdbool.h>


const int SCREEN_WIDTH = 900;
const int SCREEN_HEIGHT = 750;
const int GRID_SIZE = 16;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

struct Point
{
    int x, y;
};

struct DataSet
{
    struct Point points[20];  // Bir veri kumesinde 20'den fazla nokta olmayacagını varsayıyoruz
    int numPoints;
};
struct FilledSquare
{
    int x, y, width, height;
};

int isInsidePolygon(struct Point polygon[], int n, struct Point p)
{
    int count = 0;
    for (int i = 0, j = n - 1; i < n; j = i++)
    {
        if (((polygon[i].y >= p.y) != (polygon[j].y >= p.y)) &&
                (p.x <= (polygon[j].x - polygon[i].x) * (p.y - polygon[i].y) / (polygon[j].y - polygon[i].y) + polygon[i].x))
        {
            count++;
        }
    }
    return count % 2 != 0;
}

int checkInside(struct Point poly[], int n, struct Point p)
{
    if (isInsidePolygon(poly, n, p))
    {
        return 1;
    }
    return 0;
}


int doesSquareOverlap(struct FilledSquare filledSquares[], int numFilledSquares, int x, int y, int width, int height)
{
    for (int i = 0; i < numFilledSquares; i++)
    {
        int x1 = filledSquares[i].x;
        int y1 = filledSquares[i].y;
        int x2 = filledSquares[i].x + filledSquares[i].width;
        int y2 = filledSquares[i].y + filledSquares[i].height;

        if (x < x2 && x + width > x1 && y < y2 && y + height > y1)
        {
            // Ust uste kare algiladik
            return 1;
        }
    }
    return 0;
}


// Ici dolu bir kare cizme islevi bu fonksiyonda yapiyoruz
void drawFilledSquare(SDL_Renderer* renderer, int x1, int y1, int x2, int y, int r,int g,int b, int a)
{
    int size = abs(x2 - x1); // Boyutu x koordinatlarına gore hesaplayin

    // Kare cizim hazir fonksiyonlarını burada yazdik
    SDL_SetRenderDrawColor(renderer, r, g, b, a); // Rengin formati (R, G, B, A) (Kirmizi, Yesil, Mavi, Alpha kanali saydamlik ayari icin)
    SDL_RenderDrawRect(renderer, &(SDL_Rect)
    {
        x1, y1, size, size
    }); // Karenin dis hatti
    SDL_RenderFillRect(renderer, &(SDL_Rect)
    {
        x1, y1, size, size
    }); // Karenin ici
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Burada siyah rengine tekrardan ayarliyoruz
}


void drawGrid(SDL_Renderer* renderer)
{
    // Alpha kanali ile saydamligi degistirmek icin Blendmode'u aktif hale getiriyoruz
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // Grid rengini acık gri olarak ayarladik
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 40);

    // Yatay grid cizgilerini ciziyoruz
    for (int y = 0; y <= SCREEN_HEIGHT; y += GRID_SIZE)
    {
        SDL_RenderDrawLine(renderer, 0, y, SCREEN_WIDTH, y);
    }

    // Dikey grid cizgilerini ciziyoruz
    for (int x = 0; x <= SCREEN_WIDTH; x += GRID_SIZE)
    {
        SDL_RenderDrawLine(renderer, x, 0, x, SCREEN_HEIGHT);
    }
}


void printDataset(const struct DataSet* dataset, int choice)
{
    printf("%dB",choice);

    for (int i = 0; i < dataset->numPoints; ++i)
    {
        printf("(%d,%d)", (dataset->points[i].x)/16, (dataset->points[i].y)/16);
    }
    printf("F\n");
}
// Alani Shoelace formulu kullanarak hesapladik
void calculateArea(struct Point* polygon, int numVertices,int toplamMaliyet)
{
    double area = 0.0;

    for (int i = 0; i < numVertices - 1; ++i)
    {
        area += (polygon[i].x * polygon[i + 1].y) - (polygon[i + 1].x * polygon[i].y);
    }

    // Son islem olarak sonuncu x ve y ile baslangıc x ve y degerini isleme sokuyoruz
    area += (polygon[numVertices - 1].x * polygon[0].y) - (polygon[0].x * polygon[numVertices - 1].y);

    // Alanin mutlak degerini hesaplayin ve 2'ye bolun
    area = abs(area) / 2.0 ;

    // Orijinal alani 10 ile carparak yeni bir 'rezerv' degiskeni olusturduk
    double reserve = area * 10;
    double kar = reserve - toplamMaliyet;

    printf("Cokgenin alani: %.2f\n", area);
    printf("Rezerv degeri: %.2f\n", reserve);
    printf("Toplam kar miktari: %.2f",kar);
}
// Daha estetik bir cizim icin 16 kat buyuk olcude cizim yaptik
void scaleCoordinates(struct Point* points, int numPoints, float scaleFactor)
{
    for (int i = 0; i < numPoints; ++i)
    {
        points[i].x = (int)(points[i].x * scaleFactor);
        points[i].y = (int)(points[i].y * scaleFactor);
    }
}

// Alinan verileri islemek icin bu geri arama islevi cagrilacak
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t real_size = size * nmemb;
    char **data = (char **)userp;

    // Veriler icin bellegi yeniden tahsis ettik
    *data = (char *)realloc(*data, real_size + 1);
    if (*data)
    {
        // Alinan verileri karakter dizisine kopyaladik
        memcpy(*data, contents, real_size);
        (*data)[real_size] = '\0';
    }
    return real_size;
}

int main(int argc, char* args[])
{
    int numberOfSquares =0;
    int unitSondageMaliyet=0;
    int platformMaliyet;
    printf("Lutfen 0-10 arasinda deger alacak birim sondaj maliyetini giriniz: ");
    scanf("%d",&unitSondageMaliyet);
    if(!(unitSondageMaliyet < 10 && unitSondageMaliyet > 0))
    {
        printf("Lutfen belirtilen skalada deger giriniz.\nCıkıs yapiliyor.");
        return 0;
    }
    printf("\Lutfen birim platform maliyetini giriniz: ");
    scanf("%d",&platformMaliyet);
    printf("\n\n\n");

    int choice;
    printf("Secmek istediginiz koordinat kumesini giriniz:  ");
    scanf("%d", &choice);

    CURL *curl;
    CURLcode res;
    char *data = NULL; // Alinan verileri saklamak icin

    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, "http://zplusorg.com/prolab.txt");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        // Callback fonksiyonunu cagiriyoruz
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);

        res = curl_easy_perform(curl);
        printf("\n\nAlinan veri:\n%s\n\n", data);

        if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() hata verdi: %s\n", curl_easy_strerror(res));

        struct DataSet dataset;
        struct Point scaledPoints[10]; // Olculendirilmis koordinatlari bu dizide sakliyoruz
        printf("Seciminiz : %d\n\n", choice);

        int SatirSay = 0;
        char *line = strtok(data, "\n");

        while (line != NULL)
        {
            SatirSay++;
            if (SatirSay == choice)
            {
                struct DataSet dataset;
                char input[100];

                sscanf(line, "%*[^B]B%[^F]F", input);
                char *key = strtok(input, "(),");
                int i = 0;

                while (key != NULL && i < 10)
                {
                    int x = atoi(key);
                    key = strtok(NULL, "(),");
                    int y = atoi(key);

                    // Koordinatlari dataset struct'inda sakliyoruz
                    dataset.points[i].x = x;
                    dataset.points[i].y = y;

                    key = strtok(NULL, "(),");
                    i++;
                }
                dataset.numPoints = i; // Veri kumesindeki koordinat sayisini numPoints degiskenine atiyoruz
                break;
            }
            line = strtok(NULL, "\n");
        }

        struct Point originalPoints[20]; // Orijinal koordinatlari sakliyoruz
        memcpy(originalPoints, dataset.points, sizeof(dataset.points)); // Orijinal koordinatlari daha sonra kullanmak icin kopyaliyoruz

        // Scale the coordinates
        float scaleFactor = 16.0;  // Ihtiyacimiz oldugu olcude sekli buyutuyoruz
        scaleCoordinates(dataset.points, dataset.numPoints, scaleFactor);

        // SDL kurulumu ve renk kodları
        SDL_Init(SDL_INIT_VIDEO);
        SDL_Window* window = SDL_CreateWindow("Sondaj", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 0);

        // Ilk koordinatlari baslangıc noktasi olarak atamasini yapiyoruz
        int firstX = dataset.points[0].x;
        int firstY = dataset.points[0].y;

        SDL_RenderDrawLine(renderer, firstX, firstY,
                           dataset.points[1].x,
                           dataset.points[1].y);

        for(int index=0; index<dataset.numPoints; index++)
        {
            printf("Veri %d: (%d,%d)\n",index,dataset.points[index].x/16,dataset.points[index].y/16);
        }
        printf("\n");

        for (int shapeIndex = 0; shapeIndex < dataset.numPoints; ++shapeIndex)
        {
            // Gecerli sekli ciziyoruz
            for (int i = shapeIndex + 1; i < dataset.numPoints-1; ++i)
            {
                // Ilk noktanin gecerli noktadan farkli olup olmadigini kontrol ediyoruz
                if (firstX == dataset.points[i].x && firstY == dataset.points[i].y)
                {
                    //kod tekrar ettiyse donguyu bu turluk pas geciyoruz
                    continue;
                }
                else
                {
                    // Sadece koordinatlar ayni degilse cizim yapiliyor
                    SDL_RenderDrawLine(renderer, dataset.points[i].x, dataset.points[i].y,
                                       dataset.points[i+1].x,
                                       dataset.points[i+1].y);
                }
            }
        }

        drawGrid(renderer);

        SDL_Init(SDL_INIT_VIDEO);
        SDL_Window* window2 = SDL_CreateWindow("Sondaj Sade Hali", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        SDL_Renderer* renderer2 = SDL_CreateRenderer(window2, -1, SDL_RENDERER_ACCELERATED);

        SDL_SetRenderDrawColor(renderer2, 0, 0, 0, 0);
        SDL_RenderClear(renderer2);
        SDL_SetRenderDrawColor(renderer2, 0, 255, 0, 0);


        firstX = dataset.points[0].x;
        firstY = dataset.points[0].y;

        SDL_RenderDrawLine(renderer2, firstX, firstY,
                           dataset.points[1].x,
                           dataset.points[1].y);
        for (int shapeIndex = 0; shapeIndex < dataset.numPoints; ++shapeIndex)
        {
            // Gecerli sekli ciziyoruz
            for (int i = shapeIndex + 1; i < dataset.numPoints-1; ++i)
            {
                // Ilk noktanin gecerli noktadan farkli olup olmadigini kontrol ediyoruz
                if (firstX == dataset.points[i].x && firstY == dataset.points[i].y)
                {
                    //kod tekrar ettiyse donguyu bu turluk pas geciyoruz
                    continue;
                }
                else
                {
                    // Sadece koordinatlar ayni degilse cizim yapiliyor
                    SDL_RenderDrawLine(renderer2, dataset.points[i].x, dataset.points[i].y,
                                       dataset.points[i+1].x,
                                       dataset.points[i+1].y);
                }
            }
        }
        drawGrid(renderer2);
        int minX = dataset.points[0].x;
        int minY = dataset.points[0].y;
        int maxX = dataset.points[0].x;
        int maxY = dataset.points[0].y;

        for(int m=0; m<dataset.numPoints-1; m++)
        {
            if(minX >= dataset.points[m].x)
            {
                minX = dataset.points[m].x;
            }
        }
        for(int m=0; m<dataset.numPoints-1; m++)
        {
            if(minY >= dataset.points[m].y)
            {
                minY = dataset.points[m].y;
            }
        }
        for(int m=0; m<dataset.numPoints-1; m++)
        {
            if(maxX <= dataset.points[m].x)
            {
                maxX = dataset.points[m].x;
            }
        }
        for(int m=0; m<dataset.numPoints-1; m++)
        {
            if(maxY <= dataset.points[m].y)
            {
                maxY = dataset.points[m].y;
            }
        }

        // Cokgenler arasinda bir dongu olusturduk ve iclerindeki kareleri parsel karelerle doldurduk
        const int MAX_SQUARES = 100;
        struct FilledSquare filledSquares[MAX_SQUARES];
        for (int polygonIndex = 0; polygonIndex < dataset.numPoints; ++polygonIndex)
        {
            struct Point polygon[10];
            int numPolygonPoints = 0;

            // Koordinatlar tekrarlanana kadar gecerli cokgenin koordinatlarini ayristirdik
            do
            {
                // Gecerli noktayi cokgene ekledik
                polygon[numPolygonPoints].x = dataset.points[polygonIndex].x;
                polygon[numPolygonPoints].y = dataset.points[polygonIndex].y;

                numPolygonPoints++;

                // Bir sonraki noktaya gectik
                polygonIndex++;

                // Veri kumesinin sonuna gelirse etrafını saracak sekilde yerlesim duracak bir kosul yazdik
                if (polygonIndex >= dataset.numPoints)
                {
                    polygonIndex = 0;
                }

            }
            while (dataset.points[polygonIndex].x != polygon[0].x || dataset.points[polygonIndex].y != polygon[0].y);

            int minX = polygon[0].x;
            int minY = polygon[0].y;
            int maxX = polygon[0].x;
            int maxY = polygon[0].y;

            for(int m = 1; m < numPolygonPoints; ++m)
            {
                // Gecerli cokgenin sinirlayici iskeletini hesapladik
                if(minX >= polygon[m].x)
                    minX = polygon[m].x;
                if(minY >= polygon[m].y)
                    minY = polygon[m].y;
                if(maxX <= polygon[m].x)
                    maxX = polygon[m].x;
                if(maxY <= polygon[m].y)
                    maxY = polygon[m].y;
            }
            int numFilledSquares = 0;

            for (int j = minX; j <= maxX; j += GRID_SIZE*2)
            {
                for (int k = minY; k <= maxY; k += GRID_SIZE*2)
                {
                    // Gecerli noktanin cokgenin icinde olup olmadigini kontrol ettik
                    if (checkInside(polygon, numPolygonPoints, (struct Point)
                {
                    j, k
                })
                || checkInside(polygon, numPolygonPoints, (struct Point)
                {
                    j, k+GRID_SIZE*2
                })
                || checkInside(polygon, numPolygonPoints, (struct Point)
                {
                    j+GRID_SIZE*2, k
                })
                || checkInside(polygon, numPolygonPoints, (struct Point)
                {
                    j+GRID_SIZE*2, k+GRID_SIZE*2
                }))
                    {

                        if (checkInside(polygon, numPolygonPoints, (struct Point)
                    {
                        j, k
                    })
                    && checkInside(polygon, numPolygonPoints, (struct Point)
                    {
                        j, k+GRID_SIZE*2
                    })
                    && checkInside(polygon, numPolygonPoints, (struct Point)
                    {
                        j+GRID_SIZE*2, k
                    })
                    && checkInside(polygon, numPolygonPoints, (struct Point)
                    {
                        j+GRID_SIZE*2, k+GRID_SIZE*2
                    }))
                        {

                            if (!doesSquareOverlap(filledSquares, numFilledSquares, j, k, GRID_SIZE * 2, GRID_SIZE * 2))
                            {
                                drawFilledSquare(renderer, j, k, j + GRID_SIZE * 2, k + GRID_SIZE * 2, 0, 0, 255, 100);
                                filledSquares[numFilledSquares].x = j;
                                filledSquares[numFilledSquares].y = k;
                                filledSquares[numFilledSquares].width = GRID_SIZE * 2;
                                filledSquares[numFilledSquares].height = GRID_SIZE * 2;

                                numFilledSquares++;
                                numberOfSquares++;

                            }
                        }
                    }
                }
            }


            // Cokgenler arasinda bir dongu olusturduk ve iclerindeki kareleri parsel karelerle ikinci kez doldurduk
            for (int j = minX; j <= maxX; j += GRID_SIZE)
            {
                for (int k = minY; k <= maxY; k += GRID_SIZE)
                {
                    if (checkInside(polygon, numPolygonPoints, (struct Point)
                {
                    j, k
                })
                || checkInside(polygon, numPolygonPoints, (struct Point)
                {
                    j+GRID_SIZE, k
                })
                || checkInside(polygon, numPolygonPoints, (struct Point)
                {
                    j, k+GRID_SIZE
                })
                || checkInside(polygon, numPolygonPoints, (struct Point)
                {
                    j+GRID_SIZE, k+GRID_SIZE
                }))
                    {

                        if (checkInside(polygon, numPolygonPoints, (struct Point)
                    {
                        j, k
                    })
                    && checkInside(polygon, numPolygonPoints, (struct Point)
                    {
                        j, k+GRID_SIZE
                    })
                    && checkInside(polygon, numPolygonPoints, (struct Point)
                    {
                        j+GRID_SIZE, k
                    })
                    && checkInside(polygon, numPolygonPoints, (struct Point)
                    {
                        j+GRID_SIZE, k+GRID_SIZE
                    }))
                        {

                            if (!doesSquareOverlap(filledSquares, numFilledSquares, j, k, GRID_SIZE, GRID_SIZE))
                            {
                                drawFilledSquare(renderer, j, k, j + GRID_SIZE, k + GRID_SIZE, 0, 255, 0, 100);
                                filledSquares[numFilledSquares].x = j;
                                filledSquares[numFilledSquares].y = k;
                                filledSquares[numFilledSquares].width = GRID_SIZE;
                                filledSquares[numFilledSquares].height = GRID_SIZE;

                                numFilledSquares++;
                                numberOfSquares++;

                            }
                        }

                    }
                }
            }

            // Cokgenler arasinda bir dongu olusturduk ve iclerindeki kareleri parsel karelerle ucuncu kez doldurduk
            for (int j = minX; j <= maxX; j += GRID_SIZE/2)
            {
                for (int k = minY; k <= maxY; k += GRID_SIZE/2)
                {
                    if (checkInside(polygon, numPolygonPoints, (struct Point)
                {
                    j, k
                })
                ||checkInside(polygon, numPolygonPoints, (struct Point)
                {
                    j+GRID_SIZE/2, k
                })
                || checkInside(polygon, numPolygonPoints, (struct Point)
                {
                    j, k+GRID_SIZE/2
                })
                || checkInside(polygon, numPolygonPoints, (struct Point)
                {
                    j+GRID_SIZE/2, k+GRID_SIZE/2
                }))
                    {

                        if (checkInside(polygon, numPolygonPoints, (struct Point)
                    {
                        j, k
                    })
                    && checkInside(polygon, numPolygonPoints, (struct Point)
                    {
                        j, k+GRID_SIZE/2
                    })
                    && checkInside(polygon, numPolygonPoints, (struct Point)
                    {
                        j+GRID_SIZE/2, k
                    })
                    && checkInside(polygon, numPolygonPoints, (struct Point)
                    {
                        j+GRID_SIZE/2, k+GRID_SIZE/2
                    }))
                        {
                            if (!doesSquareOverlap(filledSquares, numFilledSquares, j, k, GRID_SIZE/2, GRID_SIZE/2))
                            {

                                drawFilledSquare(renderer, j, k, j + GRID_SIZE/2, k + GRID_SIZE/2, 255, 0, 0, 100);
                                filledSquares[numFilledSquares].x = j;
                                filledSquares[numFilledSquares].y = k;
                                filledSquares[numFilledSquares].width = GRID_SIZE/2 ;
                                filledSquares[numFilledSquares].height = GRID_SIZE/2;

                                numFilledSquares++;
                                numberOfSquares++;

                            }
                        }
                    }
                }
            }
        }
        printf("Sectiginiz veri seti: \n");
        printDataset(&dataset,choice);

        int totalCostofSondage = numberOfSquares * unitSondageMaliyet;
        int totalPlatformMaliyet = numberOfSquares * platformMaliyet;
        printf("\n\nPoligondaki kare sayisi: %d\n",numberOfSquares);
        printf("Toplam maliyet: %d\n",totalCostofSondage+totalPlatformMaliyet);

        int toplamMaliyet = totalCostofSondage + totalPlatformMaliyet;

        calculateArea(originalPoints, dataset.numPoints, toplamMaliyet);

        SDL_RenderPresent(renderer);

        SDL_RenderPresent(renderer);
        SDL_RenderPresent(renderer2);

        bool running = true;
        SDL_Event event;
        bool quit = false;
        while (!quit)
        {
            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_QUIT)
                {
                    quit = true;
                }
            }
        }


        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer2);
        SDL_DestroyWindow(window2);
        SDL_Quit();

        free(data);
        curl_easy_cleanup(curl);
    }

    return 0;
}
