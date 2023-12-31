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
    struct Point points[20];  // Assuming there won't be more than 20 points in a data set
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
            // Detected overlapping squares
            return 1;
        }
    }
    return 0;
}

// Function to draw a filled square, performs this operation here
void drawFilledSquare(SDL_Renderer* renderer, int x1, int y1, int x2, int y, int r, int g, int b, int a)
{
    int size = abs(x2 - x1); // Calculate size based on x coordinates

    // Drawing the square using built-in drawing functions here
    SDL_SetRenderDrawColor(renderer, r, g, b, a); // Color format (R, G, B, A) (Red, Green, Blue, Alpha for transparency)
    SDL_RenderDrawRect(renderer, &(SDL_Rect) { x1, y1, size, size }); // Square outline
    SDL_RenderFillRect(renderer, &(SDL_Rect) { x1, y1, size, size }); // Square fill
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Reset color to black here
}

void drawGrid(SDL_Renderer* renderer)
{
    // Activating Blendmode to change transparency with alpha channel
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // Set grid color to light gray
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 40);

    // Draw horizontal grid lines
    for (int y = 0; y <= SCREEN_HEIGHT; y += GRID_SIZE)
    {
        SDL_RenderDrawLine(renderer, 0, y, SCREEN_WIDTH, y);
    }

    // Draw vertical grid lines
    for (int x = 0; x <= SCREEN_WIDTH; x += GRID_SIZE)
    {
        SDL_RenderDrawLine(renderer, x, 0, x, SCREEN_HEIGHT);
    }
}

void printDataset(const struct DataSet* dataset, int choice)
{
    printf("%dB", choice);

    for (int i = 0; i < dataset->numPoints; ++i)
    {
        printf("(%d,%d)", (dataset->points[i].x) / 16, (dataset->points[i].y) / 16);
    }
    printf("F\n");
}

// Calculating area using the Shoelace formula
void calculateArea(struct Point* polygon, int numVertices, int totalCost)
{
    double area = 0.0;

    for (int i = 0; i < numVertices - 1; ++i)
    {
        area += (polygon[i].x * polygon[i + 1].y) - (polygon[i + 1].x * polygon[i].y);
    }

    // Finally, apply the last operation using the first and last x, y values
    area += (polygon[numVertices - 1].x * polygon[0].y) - (polygon[0].x * polygon[numVertices - 1].y);

    // Calculate the absolute value of the area and divide by 2
    area = abs(area) / 2.0;

    // Multiply the original area by 10 to create a 'reserve' variable
    double reserve = area * 10;
    double profit = reserve - totalCost;

    printf("Polygon area: %.2f\n", area);
    printf("Reserve value: %.2f\n", reserve);
    printf("Total profit amount: %.2f", profit);
}

// Perform scaling for a more aesthetic representation; drawing 16 times larger
void scaleCoordinates(struct Point* points, int numPoints, float scaleFactor)
{
    for (int i = 0; i < numPoints; ++i)
    {
        points[i].x = (int)(points[i].x * scaleFactor);
        points[i].y = (int)(points[i].y * scaleFactor);
    }
}

// Callback function to process received data
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    size_t real_size = size * nmemb;
    char** data = (char**)userp;

    // Reallocate memory for data
    *data = (char*)realloc(*data, real_size + 1);
    if (*data)
    {
        // Copy received data into a string
        memcpy(*data, contents, real_size);
        (*data)[real_size] = '\0';
    }
    return real_size;
}

int main(int argc, char* args[])
{
    int numberOfSquares = 0;
    int unitSondageMaliyet = 0;
    int platformMaliyet;

    // Get unit sondage cost from user input
    printf("Please enter a unit sondage cost between 0 and 10: ");
    scanf("%d", &unitSondageMaliyet);

    if(!(unitSondageMaliyet < 10 && unitSondageMaliyet > 0))
    {
        printf("Please enter a value on the specified scale.\nExiting.");
        return 0;
    }
    // Get platform cost from user input
    printf("Please enter the platform cost: ");
    scanf("%d", &platformMaliyet);
    printf("\n\n\n");

    int choice;
    printf("Enter the coordinate set you want to select:  ");
    scanf("%d", &choice);

    CURL *curl;
    CURLcode res;
    char *data = NULL; // To store received data

    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, "http://zplusorg.com/prolab.txt");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        // We called the Write callback function
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);

        res = curl_easy_perform(curl);
        printf("\n\nRecieved data:\n%s\n\n", data);

        if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() gave error: %s\n", curl_easy_strerror(res));

        struct DataSet dataset;
        struct Point scaledPoints[10]; // We store the dimensioned coordinates in this array
        printf("Your choice: %d\n\n", choice);

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

                    // We store the coordinates in the dataset struct
                    dataset.points[i].x = x;
                    dataset.points[i].y = y;

                    key = strtok(NULL, "(),");
                    i++;
                }
                dataset.numPoints = i; // We assign the number of coordinates in the data set to the numPoints variable
                break;
            }
            line = strtok(NULL, "\n");
        }

        struct Point originalPoints[20]; // We keep the original coordinates
        memcpy(originalPoints, dataset.points, sizeof(dataset.points)); // We copy the original coordinates for later use

        // Scale the coordinates
        float scaleFactor = 16.0;
        scaleCoordinates(dataset.points, dataset.numPoints, scaleFactor);

        // SDL initialization and color codes
        SDL_Init(SDL_INIT_VIDEO);
        SDL_Window* window = SDL_CreateWindow("Sondage Optimized", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 0);

        // We assign the first coordinates as the starting point
        int firstX = dataset.points[0].x;
        int firstY = dataset.points[0].y;

        SDL_RenderDrawLine(renderer, firstX, firstY,
                           dataset.points[1].x,
                           dataset.points[1].y);

        for(int index=0; index<dataset.numPoints; index++)
        {
            printf("Data %d: (%d,%d)\n",index,dataset.points[index].x/16,dataset.points[index].y/16);
        }
        printf("\n");

        for (int shapeIndex = 0; shapeIndex < dataset.numPoints; ++shapeIndex)
        {
            // Drawing the current shape
            for (int i = shapeIndex + 1; i < dataset.numPoints-1; ++i)
            {
                // Checking whether the first point is different from the current point
                if (firstX == dataset.points[i].x && firstY == dataset.points[i].y)
                {
                    // Skip the loop round in which the first coordinate values coincide
                    continue;
                }
                else
                {
                    // Only draw if the coordinates are not the same
                    SDL_RenderDrawLine(renderer, dataset.points[i].x, dataset.points[i].y,
                                       dataset.points[i+1].x,
                                       dataset.points[i+1].y);
                }
            }
        }

        drawGrid(renderer);

        SDL_Init(SDL_INIT_VIDEO);
        SDL_Window* window2 = SDL_CreateWindow("Sondage Plain Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
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
            // Drawing current shape
            for (int i = shapeIndex + 1; i < dataset.numPoints-1; ++i)
            {
                // Checking whether the first point is different from the current point
                if (firstX == dataset.points[i].x && firstY == dataset.points[i].y)
                {
                    // Skip the loop round in which the first coordinate values coincide
                    continue;
                }
                else
                {
                    // Only draw if the coordinates are not the same
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

        // Creating a loop between polygons and filling the squares inside them with parceled squares with blue color
        const int MAX_SQUARES = 100;
        struct FilledSquare filledSquares[MAX_SQUARES];
        for (int polygonIndex = 0; polygonIndex < dataset.numPoints; ++polygonIndex)
        {
            struct Point polygon[10];
            int numPolygonPoints = 0;

            // Parsing the coordinates of the current polygon until the coordinates are repeated
            do
            {
                // Adding current point to polygon
                polygon[numPolygonPoints].x = dataset.points[polygonIndex].x;
                polygon[numPolygonPoints].y = dataset.points[polygonIndex].y;

                numPolygonPoints++;

                // Skiping to next point
                polygonIndex++;

                // Writing a condition that will stop settling if it reaches the end of the data set, wrapping around it
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
                // Calculating the bounding skeleton of the current polygon
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
                    // Checking if the current point is inside the polygon
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


            // Creating a loop between polygons and filling the squares inside them with parceled squares with green color
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

            // Creating a loop between polygons and filling the squares inside them with parceled squares with red color
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

        // Print the selected dataset and total cost
	printf("Selected dataset: \n");
        printDataset(&dataset,choice);

	// Calculate the total cost of sondage and platform
        int totalCostofSondage = numberOfSquares * unitSondageMaliyet;
        int totalPlatformMaliyet = numberOfSquares * platformMaliyet;
	int toplamMaliyet = totalCostofSondage + totalPlatformMaliyet;

	printf("\nNumber of squares in the polygon: %d\n", numberOfSquares);
        printf("Total cost: %d\n", toplamMaliyet);

        // Calculate area using original points and total cost
	calculateArea(originalPoints, dataset.numPoints, toplamMaliyet);

        SDL_RenderPresent(renderer);

        SDL_RenderPresent(renderer);
        SDL_RenderPresent(renderer2);

        bool running = true;
        SDL_Event event;
        bool quit = false;

        // Event loop
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

	// Cleanup and free resources
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
