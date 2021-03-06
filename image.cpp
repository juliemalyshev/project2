//CSCI 5607 HW 2 - Image Conversion Instructor: S. J. Guy <sjguy@umn.edu>
//In this assignment you will load and convert between various image formats.
//Additionally, you will manipulate the stored image data by quantizing, cropping, and supressing channels

#include "image.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <iostream>

#include <fstream>
using namespace std;

/**
 * Image
 **/
Image::Image (int width_, int height_){

    assert(width_ > 0);
    assert(height_ > 0);

    width           = width_;
    height          = height_;
    num_pixels      = width * height;
    sampling_method = IMAGE_SAMPLING_POINT;

    data.raw = new uint8_t[num_pixels*4];
		int b = 0; //which byte to write to
		for (int j = 0; j < height; j++){
			for (int i = 0; i < width; i++){
				data.raw[b++] = 0;
				data.raw[b++] = 0;
				data.raw[b++] = 0;
				data.raw[b++] = 0;
			}
		}

    assert(data.raw != NULL);
}

Image::Image (const Image& src){
	width           = src.width;
	height          = src.height;
	num_pixels      = width * height;
	sampling_method = IMAGE_SAMPLING_POINT;

	data.raw = new uint8_t[num_pixels*4];

	//memcpy(data.raw, src.data.raw, num_pixels);
	*data.raw = *src.data.raw;
}

Image::Image (char* fname){

	int lastc = strlen(fname);
   int numComponents; //(e.g., Y, YA, RGB, or RGBA)
   data.raw = stbi_load(fname, &width, &height, &numComponents, 4);

	if (data.raw == NULL){
		printf("Error loading image: %s", fname);
		exit(-1);
	}

	num_pixels = width * height;
	sampling_method = IMAGE_SAMPLING_POINT;

}

Image::~Image (){
    delete data.raw;
    data.raw = NULL;
}

void Image::Write(char* fname){

	int lastc = strlen(fname);

	switch (fname[lastc-1]){
	   case 'g': //jpeg (or jpg) or png
	     if (fname[lastc-2] == 'p' || fname[lastc-2] == 'e') //jpeg or jpg
	        stbi_write_jpg(fname, width, height, 4, data.raw, 95);  //95% jpeg quality
	     else //png
	        stbi_write_png(fname, width, height, 4, data.raw, width*4);
	     break;
	   case 'a': //tga (targa)
	     stbi_write_tga(fname, width, height, 4, data.raw);
	     break;
	   case 'p': //bmp
	   default:
	     stbi_write_bmp(fname, width, height, 4, data.raw);
	}
}


void Image::Brighten (double factor){
	int x,y;
	for (x = 0 ; x < Width() ; x++){
		for (y = 0 ; y < Height() ; y++){
			Pixel p = GetPixel(x, y);
			Pixel scaled_p = p*factor;
			GetPixel(x,y) = scaled_p;
		}
	}
}

void Image::ExtractChannel(int channel) {
  int r,g,b,a;
	if(channel == 0){
    //keep red components
    for (int x = 0 ; x < Width() ; x++){
  		for (int y = 0 ; y < Height() ; y++){
        r = GetPixel(x,y).r;  //Red
        g = 0;  //Green
        b = 0; //Blue
        a = 0;  //Alpha
        Pixel red = Pixel(r,g,b,a);
        GetPixel(x,y) = red;
  		}
  	}
  }
    if(channel == 1){
      //keep green components
      for (int x = 0 ; x < Width() ; x++){
    		for (int y = 0 ; y < Height() ; y++){
          r = 0;  //Red
          g = GetPixel(x,y).g;  //Green
          b = 0; //Blue
          a = 0;  //Alpha
          Pixel green = Pixel(r,g,b,a);
          GetPixel(x,y) = green;
    		}
    	}
  }
  if(channel == 2){
    //keep blue components
    for (int x = 0 ; x < Width() ; x++){
      for (int y = 0 ; y < Height() ; y++){
        r = 0;  //Red
        g = 0;  //Green
        b = GetPixel(x,y).b; //Blue
        a = 0;  //Alpha
        Pixel blue = Pixel(r,g,b,a);
        GetPixel(x,y) = blue;
      }
    }
}
}

//TODO - HW2: Quantize the intensities stored for each pixel's values into 2^nbits possible equally-spaced values
//TODO - HW2: You may find a very helpful function in the pixel class!
void Image::Quantize (int nbits) {
	/* WORK HERE */
  int x,y;
	for (x = 0 ; x < Width() ; x++){
		for (y = 0 ; y < Height() ; y++){
      Pixel p = GetPixel(x,y);
      GetPixel(x,y) = PixelQuant(p,nbits);
		}
	}

}

Image* Image::Crop(int x, int y, int w, int h){
	/* WORK HERE */
  Image *cropped = new Image (w, h);
  int var = x;
  int vary = y;
  //  y-=vary;
	for (int i = 0; i < w ; i++){
		for (int j = 0 ; j < h; j++){
      Pixel p = GetPixel(x,y);
      cropped->GetPixel(i,j)=p;

      y++;
		}

    x++;
    y=vary;
	}

  return cropped;
}


void Image::AddNoise (double factor){
  int x,y;
  double rnoise;
  //Image copy = new Image(this);
	double range = 2*factor; 
	for (x = 0 ; x < Width() ; x++){
		for (y = 0 ; y < Height() ; y++){
      rnoise = (rand()*range)-factor; 
			Pixel p = GetPixel(x, y);
			Pixel scaled_p = p+rnoise;
			GetPixel(x,y) = scaled_p;
		}
	}
}

void Image::ChangeContrast (double factor){
  float lum;
  float averageGrey = 0;
  int i = 0;
  int x,y;
  for (x = 0 ; x < Width() ; x++){
		for (y = 0 ; y < Height() ; y++){
      lum = GetPixel(x,y).Luminance();
      averageGrey+=lum;
      i++;
		}
	}
  averageGrey/=i;
  Pixel greyP = Pixel(averageGrey,averageGrey,averageGrey,averageGrey);
  for (x = 0 ; x < Width() ; x++){
		for (y = 0 ; y < Height() ; y++){

			Pixel p = GetPixel(x, y);
			Pixel scaled_p = PixelLerp(greyP,p,factor);
			GetPixel(x,y) = scaled_p;
		}
	}
}


void Image::ChangeSaturation(double factor){
  float lum;
  float averageGrey = 0;
  int x,y;
  for (x = 0 ; x < Width() ; x++){
		for (y = 0 ; y < Height() ; y++){
      lum = GetPixel(x,y).Luminance();
      Pixel greyP = Pixel(lum,lum,lum);
			Pixel p = GetPixel(x, y);
			Pixel scaled_p = PixelLerp(greyP,p,factor);
			GetPixel(x,y) = scaled_p;
		}
	}
}

float findClosestColor(Pixel p) {
  return (float) round((float)(p.Luminance()));
} 
//For full credit, check that your dithers aren't making the pictures systematically brighter or darker
void Image::RandomDither (int nbits){
  int x,y;
  int newp = 0;
  Quantize(nbits);

  for (x = 0 ; x < Width() ; x++){
		for (y = 0 ; y < Height() ; y++){
      int random = ( rand() % ( 100 + 1 ) ) ;
			Pixel p = GetPixel(x, y);
      if(p.Luminance() > random){
        newp = 255;
      }else{
        newp = 0;
      }
      GetPixel(x,y) = Pixel(newp,newp,newp);
		}
	}
}


//This bayer method gives the quantization thresholds for an ordered dither.
//This is a 4x4 dither pattern, assumes the values are quantized to 16 levels.
//You can either expand this to a larger bayer pattern. Or (more likely), scale
//the threshold based on the target quantization levels.
static int Bayer4[4][4] ={
    {15,  7, 13,  5},
    { 3, 11,  1,  9},
    {12,  4, 14,  6},
    { 0,  8,  2, 10}
};


void Image::OrderedDither(int nbits){
	/* WORK HERE */
}

/* Error-diffusion parameters */
const double
    ALPHA = 7.0 / 16.0,
    BETA  = 3.0 / 16.0,
    GAMMA = 5.0 / 16.0,
    DELTA = 1.0 / 16.0;

void Image::FloydSteinbergDither(int nbits){
  int y,x;
  Quantize(nbits);
  for (x = 1 ; x < Width()-1 ; x++){
		for (y = 1 ; y < Height()-1 ; y++){
			Pixel p = GetPixel(x, y);

      int newpixel = findClosestColor( p ); 

      int error = p.Luminance() - newpixel;
      int neighbor1 = (int) GetPixel(x+1,y).Luminance() + ALPHA * error;
      int neighbor2 = (int) GetPixel(x-1,y+1).Luminance() + BETA * error;
      int neighbor3 = (int) GetPixel(x,y+1).Luminance() + GAMMA * error;
      int neighbor4 = (int) GetPixel(x+1,y+1).Luminance() + DELTA * error;
      GetPixel(x+1,y) = Pixel(neighbor1,neighbor1,neighbor1);
      GetPixel(x-1,y+1) = Pixel(neighbor2,neighbor2,neighbor2);
      GetPixel(x,y+1) = Pixel(neighbor3,neighbor3,neighbor3);
      GetPixel(x+1,y+1) = Pixel(neighbor4,neighbor4,neighbor4);


		}
	}
}

void Image::Blur(int n){
   // float r, g, b; //I got better results converting everything to floats, then converting back to bytes
	// Image* img_copy = new Image(*this); //This is will copying the image, so you can read the orginal values for filtering (
                                          //  ... don't forget to delete the copy!
    Image* img_copy = new Image(*this);
    float kernel[2*n+1][2*n+1] ;
    float sum = 0;
    for(int i = 0 ; i < 2*n+1; i++){
      for(int j = 0 ; j < 2*n+1; j++){
        kernel[i][j] = (1 / sqrt(pow(2*M_PI*((2*n+1)/2),2)) * exp(-(pow(i,2))/pow(2*((2*n+1)/2),2)));
        cout<<kernel[i][j]<<"\n";
        sum+=kernel[i][j];

    }
  }

    int y,x;
    int i,j = 0;
    for (x = 1 ; x < Width()-1 ; x++){
  		for (y = 1 ; y < Height()-1 ; y++){
  			//Pixel p = GetPixel(x, y);

        //guassian

            float neighbor1r = (float) GetPixel(x-1,y-1).r*kernel[0][0];
            float neighbor2r = (float) GetPixel(x,y-1).r*kernel[0][1];
            float neighbor3r = (float) GetPixel(x+1,y-1).r*kernel[0][2];
            float neighbor4r = (float) GetPixel(x-1,y).r*kernel[1][0];
            float neighbor5r = (float) GetPixel(x+1,y).r*kernel[1][1];
            float neighbor6r = (float) GetPixel(x-1,y+1).r*kernel[2][0];
            float neighbor7r = (float) GetPixel(x,y+1).r*kernel[2][1];
            float neighbor8r = (float) GetPixel(x+1,y+1).r*kernel[2][2];


        int weightedAveR = (int)ComponentClamp(((neighbor1r + neighbor2r + neighbor3r + neighbor4r + neighbor5r + neighbor6r + neighbor7r+ neighbor8r )/(sum)));
        //cout<<(float)weightedAveR<<"\n";
        float neighbor1g = (float) GetPixel(x-1,y-1).g*kernel[0][0];
        float neighbor2g = (float) GetPixel(x,y-1).g*kernel[0][1];
        float neighbor3g = (float) GetPixel(x+1,y-1).g*kernel[0][2];
        float neighbor4g = (float) GetPixel(x-1,y).g*kernel[1][0];
        float neighbor5g = (float) GetPixel(x+1,y).g*kernel[1][1];
        float neighbor6g = (float) GetPixel(x-1,y+1).g*kernel[2][0];
        float neighbor7g = (float) GetPixel(x,y+1).g*kernel[2][1];
        float neighbor8g = (float) GetPixel(x+1,y+1).g*kernel[2][2];


        int weightedAveG = (int)ComponentClamp(((neighbor1g + neighbor2g + neighbor3g + neighbor4g + neighbor5g + neighbor6g + neighbor7g+ neighbor8g )/(sum)));

        float neighbor1b = (float) GetPixel(x-1,y-1).b*kernel[0][0];
        float neighbor2b = (float) GetPixel(x,y-1).b*kernel[0][1];
        float neighbor3b = (float) GetPixel(x+1,y-1).b*kernel[0][2];
        float neighbor4b = (float) GetPixel(x-1,y).b*kernel[1][0];
        float neighbor5b = (float) GetPixel(x+1,y).b*kernel[1][1];
        float neighbor6b = (float) GetPixel(x-1,y+1).b*kernel[2][0];
        float neighbor7b = (float) GetPixel(x,y+1).b*kernel[2][1];
        float neighbor8b = (float) GetPixel(x+1,y+1).b*kernel[2][2];


        int weightedAveB = (int)ComponentClamp(((neighbor1b + neighbor2b + neighbor3b + neighbor4b + neighbor5b + neighbor6b + neighbor7b+ neighbor8b )/(sum)));


        GetPixel(x,y) = Pixel(weightedAveR,weightedAveG,weightedAveB);

  		}
  	}
    img_copy->~Image();
}

void Image::Sharpen(int n){
  int x,y;
  Image* img_copy = new Image(*this);
  img_copy->Blur(n);
  for (x = 0 ; x < Width() ; x++){
    for (y = 0 ; y < Height() ; y++){
      GetPixel(x,y) = PixelLerp(img_copy->GetPixel(x,y),GetPixel(x,y), 1.5);
    }
  }
}

void Image::EdgeDetect(){
  Image* img_copy = new Image(*this);
  int threshold = 70;
  int x,y;
  for (x = 1 ; x < Width()-1 ; x++){
    for (y = 1 ; y < Height()-1 ; y++){
      float neighbor1 = (float) img_copy->GetPixel(x-1,y-1).Luminance();
      float neighbor2 = (float) img_copy->GetPixel(x,y-1).Luminance();
      float neighbor3 = (float) img_copy->GetPixel(x+1,y-1).Luminance();
      float neighbor4 = (float) img_copy->GetPixel(x-1,y).Luminance();
      float neighbor5 = (float) img_copy->GetPixel(x+1,y).Luminance();
      float neighbor6 = (float) img_copy->GetPixel(x-1,y+1).Luminance();
      float neighbor7 = (float) img_copy->GetPixel(x,y+1).Luminance();
      float neighbor8 = (float) img_copy->GetPixel(x+1,y+1).Luminance();

      if(GetPixel(x,y).Luminance()-neighbor1 >threshold){
        GetPixel(x-1,y-1) = Pixel(255,255,255);
        GetPixel(x,y) = Pixel(0,0,0);
      }
      cout<<GetPixel(x,y).Luminance()-neighbor1<<"\n";
      if(GetPixel(x,y).Luminance()-neighbor2 >threshold){
        GetPixel(x,y-1) = Pixel(255,255,255);
        GetPixel(x,y) = Pixel(0,0,0);
      }
      if(GetPixel(x,y).Luminance()-neighbor3 >threshold){
        GetPixel(x+1,y-1) = Pixel(255,255,255);
        GetPixel(x,y) = Pixel(0,0,0);
      }
      if(GetPixel(x,y).Luminance()-neighbor4 >threshold){
        GetPixel(x-1,y) = Pixel(255,255,255);
        GetPixel(x,y) = Pixel(0,0,0);
      }
      if(GetPixel(x,y).Luminance()-neighbor5 >threshold){
        GetPixel(x+1,y) = Pixel(255,255,255);
        GetPixel(x,y) = Pixel(0,0,0);
      }
      if(GetPixel(x,y).Luminance()-neighbor6 >threshold){
        GetPixel(x-1,y+1) = Pixel(255,255,255);
        GetPixel(x,y) = Pixel(0,0,0);
      }
      if(GetPixel(x,y).Luminance()-neighbor7 >threshold){
        GetPixel(x,y+1) = Pixel(255,255,255);
        GetPixel(x,y) = Pixel(0,0,0);
      }
      if(GetPixel(x,y).Luminance()-neighbor8 >threshold){
        GetPixel(x+1,y+1) = Pixel(255,255,255);
        GetPixel(x,y) = Pixel(0,0,0);
      }
      GetPixel(x,y) = Pixel(0,0,0);
    }
  }

}

Image* Image::Scale(double sx, double sy){
  Image* img_copy = new Image(*this);
  int x,y;
	float r = max(1.0/sx,1.0/sy);
  for (x = 0 ; x < Width() ; x++){
    for (y = 0 ; y < Height() ; y++){
      float u = x/sx;
      float v = y/sy;
      img_copy->SetPixel(x,y,Sample(u,v));
    }
  }
  return img_copy;
}

Image* Image::Rotate(double angle){
  Image* img_copy = new Image(*this);
  int x,y;
  for (x = 0 ; x < Width() ; x++){
    for (y = 0 ; y < Height() ; y++){
      float u = x*cos(-angle) - y * sin(-angle);
      float v = x*sin(-angle) + y * cos(-angle);
      img_copy->GetPixel(x,y) = Sample(u,v);
    }
  }
	return img_copy;
}

void Image::Fun(){
	/* WORK HERE */
}

/**
 * Image Sample
 **/
void Image::SetSamplingMethod(int method){
   assert((method >= 0) && (method < IMAGE_N_SAMPLING_METHODS));
   sampling_method = method;
}


Pixel Image::Sample (double u, double v){
   if(sampling_method == 0){
     return Pixel();
   }
   if(sampling_method == 1){
     return Pixel();
   }
   if(sampling_method == 2){
     return Pixel();
   }
   return Pixel();
}
