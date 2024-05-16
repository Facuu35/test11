#include "image.h"

#include <stdio.h>

int main() {
  // Allocate memory for struct
  Bitmap original_image;

  // Load in data from image into struct
  if (load_img("./original.bmp", &original_image) == LOAD_ERROR) {
    printf("Unable to load image. Exiting...\n");
    return 1;
  }

  // Process the image (Green channel removal)
  remove_color_channel(GREEN_CHANNEL, &original_image);
  if (save_img("green_mask.bmp", &original_image) == SAVE_ERROR) {
    printf("Unable to save image. Exiting...\n");
    return 1;
  }

  // Reset the image data to its original state
  reset_pixel_data(&original_image);

  // Process the image (Blue channel removal)
  remove_color_channel(BLUE_CHANNEL, &original_image);
  if (save_img("blue_mask.bmp", &original_image) == SAVE_ERROR) {
    printf("Unable to save image. Exiting...\n");
    return 1;
  }

  // Reset the image data to its original state
  reset_pixel_data(&original_image);

  // Process the image (Red channel removal)
  remove_color_channel(RED_CHANNEL, &original_image);
  if (save_img("red_mask.bmp", &original_image) == SAVE_ERROR) {
    printf("Unable to save image. Exiting...\n");
    return 1;
  }

  // Reset the image data to its original state (important for grayscale and filter)
  reset_pixel_data(&original_image);

  // Process the image (Grayscale conversion)
  grayscale(&original_image);
  if (save_img("grayscale.bmp", &original_image) == SAVE_ERROR) {
    printf("Unable to save image. Exiting...\n");
    return 1;
  }

  // Process the image (OR filter)
  or_filter(&original_image);
  if (save_img("or_filter.bmp", &original_image) == SAVE_ERROR) {
    printf("Unable to save image. Exiting...\n");
    return 1;
  }

  return 0;
}
