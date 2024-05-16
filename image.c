/*
This is the driver file for the image library. All function definitions live in here
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


#include "image.h"

// Gets the pointer to the pixel data. This data can be modified. The save_img
// function uses this data to write a new image.
uint8_t *get_pxl_data(Bitmap *bmp) {
    return bmp->pxl_data;
}

// Gets the pointer to the pixel data of the original image. This data should
// never be modified!
uint8_t *get_original_pxl_data(Bitmap *bmp) {
    return bmp->pxl_data_cpy;
}

// --------------------------------------------------------------------------
// Image loading/saving functions
// --------------------------------------------------------------------------

uint8_t load_img(char *filepath, Bitmap *bmp) {
    // Opens file if it exists
    if (!(bmp->img = fopen(filepath, "r"))) {
        return LOAD_ERROR;
    }

    // Read in the bitmap file header
    fread(bmp->file_header, sizeof(uint8_t), BMP_FILE_HEADER_SIZE, bmp->img);

    // Get filesize
    bmp->file_size = (bmp->file_header[5] << 8 * 3) | (bmp->file_header[4] << 8 * 2) |
                     (bmp->file_header[3] << 8 * 1) | (bmp->file_header[2]);

    // Get start location of pixel data
    bmp->pxl_data_offset = (bmp->file_header[13] << 8 * 3) | (bmp->file_header[12] << 8 * 2) |
                           (bmp->file_header[11] << 8 * 1) | (bmp->file_header[10]);

    // Get DIB header data
    uint8_t dib_header_size = bmp->pxl_data_offset - BMP_FILE_HEADER_SIZE;
    bmp->dib_header = (uint8_t *)malloc((dib_header_size) * sizeof(uint8_t));
    fread(bmp->dib_header, sizeof(uint8_t), dib_header_size, bmp->img);

    // Get image width
    bmp->img_width = (bmp->dib_header[7] << 8 * 3) | (bmp->dib_header[6] << 8 * 2) |
                     (bmp->dib_header[5] << 8 * 1) | (bmp->dib_header[4]);

    // Get image height
    bmp->img_height = (bmp->dib_header[11] << 8 * 3) | (bmp->dib_header[10] << 8 * 2) |
                      (bmp->dib_header[9] << 8 * 1) | (bmp->dib_header[8]);

    // Get image data
    bmp->pxl_data_size = bmp->img_width * bmp->img_height * 3;
    bmp->pxl_data = (uint8_t *)malloc(sizeof(uint8_t) * bmp->pxl_data_size);
    fread(bmp->pxl_data, sizeof(uint8_t), bmp->pxl_data_size, bmp->img);

    // Create copy of image data for reset
    bmp->pxl_data_cpy = (uint8_t *)malloc(sizeof(uint8_t) * bmp->pxl_data_size);
    bmp->pxl_data_cpy = memcpy(bmp->pxl_data_cpy, bmp->pxl_data, bmp->pxl_data_size);

    // Close the file
    fclose(bmp->img);

    return LOAD_SUCCESS;
}

void reset_pixel_data(Bitmap *bmp) {
    bmp->pxl_data = memcpy(bmp->pxl_data, bmp->pxl_data_cpy, bmp->pxl_data_size);
}

uint8_t save_img(char *imgname, Bitmap *bmp) {
    FILE *save_img;
    if (!(save_img = fopen(imgname, "w"))) {
        fprintf(stderr, "Could not save the file with that name. Change the name and try again.\n");
        return SAVE_ERROR;
    }

    // Write BMP header
    fwrite(bmp->file_header, sizeof(uint8_t), BMP_FILE_HEADER_SIZE, save_img);

    // Write DIB header
    fwrite(bmp->dib_header, sizeof(uint8_t), bmp->pxl_data_offset - BMP_FILE_HEADER_SIZE, save_img);

    // Write pixel data
    fwrite(bmp->pxl_data, sizeof(uint8_t), bmp->pxl_data_size, save_img);

    // Close the file
    fclose(save_img);
    return SAVE_SUCCESS;
}

// --------------------------------------------------------------------------
// Image manipulation functions
// --------------------------------------------------------------------------
void remove_color_channel(Color color, Bitmap *bmp) {
    // Iterate through each pixel
    for (uint32_t y = 0; y < bmp->img_height; y++) {
        for (uint32_t x = 0; x < bmp->img_width; x++) {
            // Calculate pixel offset based on width and channel
            uint8_t *pixel = bmp->pxl_data + (y * bmp->img_width + x) * 3;
            switch (color) {
                case RED_CHANNEL:
                    // Set red channel to 0
                    pixel[2] = 0;
                    break;
                case GREEN_CHANNEL:
                    // Set green channel to 0
                    pixel[1] = 0;
                    break;
                case BLUE_CHANNEL:
                    // Set blue channel to 0
                    pixel[0] = 0;
                    break;
                default:
                    break;
            }
        }
    }
}

void grayscale(Bitmap *bmp) {
    // Iterate through each pixel
    for (uint32_t y = 0; y < bmp->img_height; y++) {
        for (uint32_t x = 0; x < bmp->img_width; x++) {
            // Calculate pixel offset
            uint8_t *pixel = bmp->pxl_data + (y * bmp->img_width + x) * 3;
            // Calculate grayscale value
            uint8_t gray_value = (uint8_t)(0.299 * pixel[2] + 0.587 * pixel[1] + 0.114 * pixel[0]);
            // Set RGB values to grayscale value
            pixel[0] = gray_value;
            pixel[1] = gray_value;
            pixel[2] = gray_value;
        }
    }
}

void or_filter(Bitmap *bmp) {
    // Iterate through each pixel
    for (uint32_t y = 1; y < bmp->img_height - 1; y++) {
        for (uint32_t x = 0; x < bmp->img_width; x++) {
            // Calculate pixel offset
            uint8_t *pixel = bmp->pxl_data + (y * bmp->img_width + x) * 3;
            uint8_t *top_pixel = pixel - bmp->img_width * 3;
            uint8_t *bottom_pixel = pixel + bmp->img_width * 3;
            // Apply OR filter to each color channel
            for (int i = 0; i < 3; i++) {
                pixel[i] |= top_pixel[i] | bottom_pixel[i];
            }
        }
    }
    // Handle top and bottom rows separately
    for (uint32_t x = 0; x < bmp->img_width; x++) {
        uint8_t *pixel = bmp->pxl_data + x * 3;
        uint8_t *bottom_pixel = bmp->pxl_data + (bmp->img_height - 1) * bmp->img_width * 3 + x * 3;
        for (int i = 0; i < 3; i++) {
            pixel[i] |= bottom_pixel[i];
        }
    }
}
