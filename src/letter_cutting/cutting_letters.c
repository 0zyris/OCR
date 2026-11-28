#include <SDL2/SDL.h>
#include "layout.h"
#include "cutting_letters.h"
#include "cutting_tools.h"

// Tighten a frame around the black pixels it contains
void crop_box(SDL_Surface *surface, int *xmin, int *xmax, int *ymin,
    int *ymax) {
    if (SDL_MUSTLOCK(surface)) SDL_LockSurface(surface);
    Uint32 *pixels = (Uint32 *)surface->pixels;
    SDL_PixelFormat *format = surface->format;
    Uint8 r, g, b;

    int new_ymin = surface->h, new_ymax = 0;
    int new_xmin = surface->w, new_xmax = 0;
    int found = 0;

    for (int y = *ymin; y <= *ymax; y++) {
        for (int x = *xmin; x <= *xmax; x++) {
            if (x >= 0 && x < surface->w && y >= 0 && y < surface->h) {
                SDL_GetRGB(pixels[y * surface->w + x], format, &r, &g, &b);
                if (r == 0) { // If we find ink
                    if (x < new_xmin) new_xmin = x;
                    if (x > new_xmax) new_xmax = x;
                    if (y < new_ymin) new_ymin = y;
                    if (y > new_ymax) new_ymax = y;
                    found = 1;
                }
            }
        }
    }
    
    // Update of coordinates
    if (found) {
        *xmin = new_xmin; *xmax = new_xmax;
        *ymin = new_ymin; *ymax = new_ymax;
    }

    if (SDL_MUSTLOCK(surface)) SDL_UnlockSurface(surface);
}

//allows the green to be removed in the case without a grid
void remove_green(SDL_Surface *surface, int xmin, int xmax, int ymin,
        int ymax) {
    if (SDL_MUSTLOCK(surface)) SDL_LockSurface(surface);
    Uint32 *pixels = (Uint32 *)surface->pixels;
    SDL_PixelFormat *format = surface->format;
    Uint32 green = SDL_MapRGB(format, 0, 255, 0);
    Uint32 white = SDL_MapRGB(format, 255, 255, 255);
    for (int y = ymin; y <= ymax; y++) {
        if (y < 0 || y >= surface->h) continue;
        for (int x = xmin; x <= xmax; x++) {
            if (x < 0 || x >= surface->w) continue;
            if (pixels[y * surface->w + x] == green) {
                pixels[y * surface->w + x] = white;
            }
        }
    }

    if (SDL_MUSTLOCK(surface)) SDL_UnlockSurface(surface);
}

SDL_Surface *normalize_letter_surface(SDL_Surface *subsurface) {
    if(!subsurface) {
        return NULL;
    }
    SDL_Surface *dest = SDL_CreateRGBSurfaceWithFormat(
        0,
        28,
        28,
        subsurface->format->BitsPerPixel,
        subsurface->format->format
    );
    if(!dest) {
        return NULL;
    }
    SDL_FillRect(dest, NULL, SDL_MapRGB(dest->format, 255, 255, 255));
    double scale_x = 28.0/subsurface->w;
    double scale_y = 28.0/subsurface->h;
    double scale = (scale_x < scale_y) ? scale_x : scale_y;
    int new_w = (int)(subsurface->w * scale);
    int new_h = (int)(subsurface->h * scale);
    SDL_Rect dst_rect = {
        .x = (28 - new_w) / 2,
        .y = (28 - new_h) / 2,
        .w = new_w,
        .h = new_h
    };
    SDL_BlitScaled(subsurface, NULL, dest, &dst_rect);
    return dest;
}

SDL_Surface *create_letter_surface(SDL_Surface *surface, int x, int y,
        int w, int h) {
    SDL_Rect rectangle = {
        .x = x,
        .y = y,
        .w = w,
        .h = h
    };
    SDL_Surface *subsurface = SDL_CreateRGBSurfaceWithFormat(0, w, h,
        surface->format->BitsPerPixel, surface->format->format);
    if(!subsurface) {
        return NULL;
    }
    SDL_BlitSurface(surface, &rectangle, subsurface, NULL);
    remove_green(subsurface, -1, w+1,-1, h+1);
    SDL_Surface *normalized = normalize_letter_surface(subsurface);
    SDL_FreeSurface(subsurface);
    return normalized;
}

int is_black(SDL_Surface *surface, int x, int y)
{
    if (x < 0 || x >= surface->w || y < 0 || y >= surface->h) {
        return 0;
    }
    Uint32 *pixels = (Uint32 *)surface->pixels;
    Uint8 r, g, b;
    SDL_GetRGB( pixels[y * surface->w + x], surface->format,
        &r, &g, &b);
    return ((r + g + b) < 150);
}

void bfs_component(SDL_Surface *surface,int start_x,int start_y, char *visited,
                   int *xmin,int *xmax,int *ymin,int *ymax,int word_xmin,
                   int word_xmax,int word_ymin,int word_ymax)
{
    int max_size = surface->w * surface->h;
    struct Point *queue = malloc(max_size * sizeof(struct Point));
    int front = 0;
    int back = 0;
    queue[back++] = (struct Point){start_x, start_y};
    visited[start_y * surface->w + start_x] = 1;
    *xmin = *xmax = start_x;
    *ymin = *ymax = start_y;
    while (front < back) {
        struct Point p = queue[front++];
        if (p.x < *xmin)
            *xmin = p.x;
        if (p.x > *xmax)
            *xmax = p.x;
        if (p.y < *ymin)
            *ymin = p.y;
        if (p.y > *ymax)
            *ymax = p.y;
        static const int dx[4] = {1, -1, 0, 0};
        static const int dy[4] = {0, 0, 1, -1};
        for (int i = 0; i < 4; i++)
        {
            int nx = p.x + dx[i];
            int ny = p.y + dy[i];
            if (nx < word_xmin || nx > word_xmax ||
                ny < word_ymin || ny > word_ymax)
                continue;
            if (visited[ny * surface->w + nx])
                continue;
            if (!is_black(surface, nx, ny))
                continue;
            visited[ny * surface->w + nx] = 1;
            queue[back++] = (struct Point){nx, ny};
        }
    }
    free(queue);
}

// Isolate the letters from the grid and frame them in purple
// as closely as possible.
struct letters_surface *segment_letters(SDL_Surface *surface, int xmin,
        int xmax, int ymin, int ymax, int grid) {
    struct letters_surface *head_letters = NULL;
    int size = surface->w * surface->h;
    char *visited = calloc(size, sizeof(char));
    if (!visited)
        return NULL;
    if (SDL_MUSTLOCK(surface))
        SDL_LockSurface(surface);
    for (int y = ymin; y <= ymax; y++) {
        for (int x = xmin; x <= xmax; x++) {
            if (visited[y * surface->w + x])
                continue;
            if (!is_black(surface, x, y))
                continue;
            int lxmin, lxmax, lymin, lymax;
            bfs_component(surface, x, y, visited, &lxmin, &lxmax, &lymin,
                &lymax, xmin, xmax, ymin, ymax);
            int area = (lxmax - lxmin + 1)*(lymax - lymin + 1);
            if (area < 20)
                continue;
            SDL_Surface *subsurface = create_letter_surface(surface, lxmin,
                    lymin, lxmax - lxmin + 1, lymax - lymin + 1);
            if (!subsurface)
                continue;
            struct letters_surface *new_letter =
                initialize_letters_surface(subsurface);
            if (!new_letter) {
                SDL_FreeSurface(subsurface);
                free(visited);
                if (SDL_MUSTLOCK(surface))
                    SDL_UnlockSurface(surface);
                return NULL;
            }
            insert_letters_surface(&head_letters, new_letter);
            if (grid) {
                draw_rectangle(surface,lxmin,lxmax,lymin,lymax,128,128,0);
            }
            else {
                draw_rectangle(surface,lxmin,lxmax,lymin,lymax,128,0,128);
            }
        }
    }
    if (SDL_MUSTLOCK(surface))
        SDL_UnlockSurface(surface);
    free(visited);
    return head_letters;
}

// Isolate each word from the list and frame it in green as closely as possible
struct head_surface *segment_word_list(SDL_Surface *surface, int xmin,
        int xmax, int ymin, int ymax, int grid) {
    int in_word = 0;
    int word_ymin = 0;
    int threshold = 2; // Tolerance
    struct head_surface *head = NULL;
    for (int y = ymin; y <= ymax; y++) {
        int black_pixels = 0;
        if (SDL_MUSTLOCK(surface)) SDL_LockSurface(surface);
        Uint32 *pixels = (Uint32 *)surface->pixels;
        SDL_PixelFormat *format = surface->format;
        Uint8 r, g, b;

        for (int x = xmin; x <= xmax; x++) {
            if (x >= 0 && x < surface->w && y >= 0 && y < surface->h) {
                SDL_GetRGB(pixels[y * surface->w + x], format, &r, &g, &b);
                if (r == 0) black_pixels++;
            }
        }
        if (SDL_MUSTLOCK(surface)) SDL_UnlockSurface(surface);

        if (black_pixels > threshold) {
            if (!in_word) {
                in_word = 1;
                word_ymin = y;
            }
        } else {
            if (in_word) {
                in_word = 0;
                int w_xmin = xmin, w_xmax = xmax;
                int w_ymin = word_ymin, w_ymax = y - 1;
                
                // Tighten the green frame around the precise word
                crop_box(surface, &w_xmin, &w_xmax, &w_ymin, &w_ymax);
                w_xmin--, w_ymin--; 
                w_xmax++, w_ymax++; // Adding a margin of 1 pixel
                struct letters_surface *head_letter = segment_letters(surface,
                    w_xmin, w_xmax, w_ymin, w_ymax, grid);
                draw_rectangle(surface, w_xmin, w_xmax, w_ymin, w_ymax, 0, 
                    255, 0);
                if(!head_letter) {
                    free_head_surface(head);
                    return NULL; //error case
                }
                struct head_surface *new = 
                    initialize_head_surface(head_letter);
                insert_head_surface(&head, new);
            }
        }
    }
    
    // Last word
    if (in_word) {
        int w_xmin = xmin, w_xmax = xmax;
        int w_ymin = word_ymin, w_ymax = ymax;
        
        crop_box(surface, &w_xmin, &w_xmax, &w_ymin, &w_ymax);
        w_xmin--, w_ymin--; 
        w_xmax++, w_ymax++; // Adding a margin of 1 pixel
        struct letters_surface *head_letter2 = segment_letters(surface, w_xmin,
            w_xmax, w_ymin, w_ymax, grid);
        draw_rectangle(surface, w_xmin, w_xmax, w_ymin, w_ymax, 0, 255, 0);
        if(!head_letter2) {
            free_head_surface(head);
            return NULL; //error case
        }
        struct head_surface *new = initialize_head_surface(head_letter2);
        insert_head_surface(&head, new);
    }
    return head;
}

// test whether the word mélé has a grid using the histogram
int has_grid(int xmin, int xmax, int ymin, int ymax, int* vert_hist,
        int* horiz_hist)
{
    int width = xmax - xmin + 1;
    int height = ymax - ymin + 1;
    int vertical_threshold = (height * 8) / 10;
    int horizontal_threshold = (width * 8) / 10;
    int vertical_lines = 0;
    int horizontal_lines = 0;
    int in_vertical_line = 0;
    for (int x = xmin; x <= xmax; x++) {
        if (vert_hist[x] >= vertical_threshold) {
            if (!in_vertical_line) {
                vertical_lines++;
                in_vertical_line = 1;
            }
        } else {
            in_vertical_line = 0;
        }
    }
    int in_horizontal_line = 0;
    for (int y = ymin; y <= ymax; y++) {
        if (horiz_hist[y] >= horizontal_threshold) {
            if (!in_horizontal_line) {
                horizontal_lines++;
                in_horizontal_line = 1;
            }
        } else {
            in_horizontal_line = 0;
        }
    }
    return (vertical_lines >= 2 && horizontal_lines >= 2);
}

//calculation of the max shaver at the edge of the grid
int compute_cell_margin(int xmin, int xmax, int ymin, int ymax, int *vert_hist,
        int *horiz_hist, double threshold) {
    int vert_limit = (int)((ymax-ymin+1)*threshold);
    int horiz_limit = (int)((xmax-xmin+1)*threshold);
    int max_thickness = 0;
    int thickness = 0;
    for (int x = xmin; x <= xmax; x++) {
        if (vert_hist[x] >= vert_limit) {
            thickness++;
        }
        else {
            if (thickness > max_thickness)
                max_thickness = thickness;

            thickness = 0;
        }
    }
    if (thickness > max_thickness)
        max_thickness = thickness;
    thickness = 0;
    for (int y = ymin; y <= ymax; y++) {
        if (horiz_hist[y] >= horiz_limit) {
            thickness++;
        }
        else {
            if (thickness > max_thickness)
                max_thickness = thickness;
            thickness = 0;
        }
    }
    if (thickness > max_thickness)
        max_thickness = thickness;
    return max_thickness+1;
}

// Isolate the letters from the grid and frame them in yellow
struct head_surface *segment_grid_letters_with_histogram(SDL_Surface *surface,
        int xmin, int xmax, int ymin, int ymax, int* vert_hist,
        int* horiz_hist) {
    if (!has_grid(xmin, xmax, ymin, ymax, vert_hist, horiz_hist)) {
        struct head_surface *head = segment_word_list(surface, xmin, xmax,
            ymin, ymax, 1);
        remove_green(surface, xmin-1, xmax+1, ymin-1, ymax+1);
        return head;
    } else {
        //case with additional noise or pixel
        const double threshold = 0.65;
        const int LINE_GAP = 5;
        const int CELL_MARGIN = compute_cell_margin(xmin, xmax, ymin, ymax,
            //number of pixels to remove at the edge
            vert_hist, horiz_hist, threshold);
        int MAX_LINES = 256;
        int vert_lines[MAX_LINES];
        int horiz_lines[MAX_LINES];
        int nb_vert = 0;
        int nb_horiz = 0;
        int vert_limit = (int)((ymax-ymin+1)*threshold);
        int horiz_limit = (int)((xmax-xmin+1)*threshold);
        for (int x = xmin; x <= xmax; x++) {
            if (vert_hist[x] >= vert_limit) {
                if (nb_vert == 0 || x - vert_lines[nb_vert - 1] > LINE_GAP) {
                    if (nb_vert < MAX_LINES) {
                        vert_lines[nb_vert++] = x;
                    }
                }
            }
        }
        for (int y = ymin; y <= ymax; y++) {
            if (horiz_hist[y] >= horiz_limit) {
                if (nb_horiz == 0 || y - horiz_lines[nb_horiz - 1] > LINE_GAP){
                    if (nb_horiz < MAX_LINES) {
                        horiz_lines[nb_horiz++] = y;
                    }
                }
            }
        }
        struct head_surface *head = NULL;
        for (int row = 0; row < nb_horiz - 1; row++) {
            struct letters_surface *head_letters = NULL;
            for (int col = 0; col < nb_vert - 1; col++) {
                int l_xmin = vert_lines[col] + CELL_MARGIN;
                int l_xmax = vert_lines[col + 1] - CELL_MARGIN;
                int l_ymin = horiz_lines[row] + CELL_MARGIN;
                int l_ymax = horiz_lines[row + 1] - CELL_MARGIN;
                crop_box(surface, &l_xmin, &l_xmax, &l_ymin, &l_ymax);
                if (l_xmin>l_xmax || l_ymin>l_ymax) continue;
                SDL_Surface *subsurface = create_letter_surface(surface,
                    l_xmin, l_ymin, l_xmax-l_xmin+1, l_ymax-l_ymin+1);
                struct letters_surface *new_letter = 
                    initialize_letters_surface(subsurface);
                if(!new_letter) {
                    free_head_surface(head);
                    SDL_FreeSurface(subsurface);
                    return NULL; //error case
                }
                insert_letters_surface(&head_letters, new_letter);
                draw_rectangle(surface, l_xmin, l_xmax, l_ymin, l_ymax, 128,
                    128, 0);
            }
            struct head_surface *new = initialize_head_surface(head_letters);
            insert_head_surface(&head, new);
        }
        return head;
    }
}

void save_surface(struct head_surface *head, int grid) {
    struct head_surface *temp = head;
    int row = 0;
    while(temp) {
        int col = 0;
        struct letters_surface *letters = temp->head;
        while(letters) {
            char path[100];
            if(grid) {
                snprintf(path, sizeof(path), 
                    "image/image_storage/grid/grid%d_%d.png", row, col);
                SDL_SaveBMP(letters->surface, path);
            } else {
                snprintf(path, sizeof(path), 
                    "image/image_storage/word/word%d_%d.png", row, col);
                SDL_SaveBMP(letters->surface, path);
            }
            
            col++;
            letters = letters->next;
        }
        row++;
        temp = temp->next;
    }
    free_head_surface(head);
}
