#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "GL/glew.h"
#include "../extra/picopng.h"
#include "image.h"
#include "utils.h"
#include "camera.h"
#include "mesh.h"

Image::Image() {
	width = 0; height = 0;
	pixels = NULL;
}

Image::Image(unsigned int width, unsigned int height)
{
	this->width = width;
	this->height = height;
	pixels = new Color[width*height];
	memset(pixels, 0, width * height * sizeof(Color));
}

// Copy constructor
Image::Image(const Image& c)
{
	pixels = NULL;
	width = c.width;
	height = c.height;
	bytes_per_pixel = c.bytes_per_pixel;
	if(c.pixels)
	{
		pixels = new Color[width*height];
		memcpy(pixels, c.pixels, width*height*bytes_per_pixel);
	}
}

// Assign operator
Image& Image::operator = (const Image& c)
{
	if(pixels) delete pixels;
	pixels = NULL;

	width = c.width;
	height = c.height;
	bytes_per_pixel = c.bytes_per_pixel;

	if(c.pixels)
	{
		pixels = new Color[width*height*bytes_per_pixel];
		memcpy(pixels, c.pixels, width*height*bytes_per_pixel);
	}
	return *this;
}

Image::~Image()
{
	if(pixels) 
		delete pixels;
}

void Image::Render()
{
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glDrawPixels(width, height, bytes_per_pixel == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, pixels);
}

// Change image size (the old one will remain in the top-left corner)
void Image::Resize(unsigned int width, unsigned int height)
{
	Color* new_pixels = new Color[width*height];
	unsigned int min_width = this->width > width ? width : this->width;
	unsigned int min_height = this->height > height ? height : this->height;

	for(unsigned int x = 0; x < min_width; ++x)
		for(unsigned int y = 0; y < min_height; ++y)
			new_pixels[ y * width + x ] = GetPixel(x,y);

	delete pixels;
	this->width = width;
	this->height = height;
	pixels = new_pixels;
}

// Change image size and scale the content
void Image::Scale(unsigned int width, unsigned int height)
{
	Color* new_pixels = new Color[width*height];

	for(unsigned int x = 0; x < width; ++x)
		for(unsigned int y = 0; y < height; ++y)
			new_pixels[ y * width + x ] = GetPixel((unsigned int)(this->width * (x / (float)width)), (unsigned int)(this->height * (y / (float)height)) );

	delete pixels;
	this->width = width;
	this->height = height;
	pixels = new_pixels;
}

Image Image::GetArea(unsigned int start_x, unsigned int start_y, unsigned int width, unsigned int height)
{
	Image result(width, height);
	for(unsigned int x = 0; x < width; ++x)
		for(unsigned int y = 0; y < height; ++x)
		{
			if( (x + start_x) < this->width && (y + start_y) < this->height) 
				result.SetPixel( x, y, GetPixel(x + start_x,y + start_y) );
		}
	return result;
}

void Image::FlipY()
{
	int row_size = bytes_per_pixel * width;
	Uint8* temp_row = new Uint8[row_size];
#pragma omp simd
	for (int y = 0; y < height * 0.5; y += 1)
	{
		Uint8* pos = (Uint8*)pixels + y * row_size;
		memcpy(temp_row, pos, row_size);
		Uint8* pos2 = (Uint8*)pixels + (height - y - 1) * row_size;
		memcpy(pos, pos2, row_size);
		memcpy(pos2, temp_row, row_size);
	}
	delete[] temp_row;
}

bool Image::LoadPNG(const char* filename, bool flip_y)
{
	std::string sfullPath = absResPath(filename);
	std::ifstream file(sfullPath, std::ios::in | std::ios::binary | std::ios::ate);

	// Get filesize
	std::streamsize size = 0;
	if (file.seekg(0, std::ios::end).good()) size = file.tellg();
	if (file.seekg(0, std::ios::beg).good()) size -= file.tellg();

	if (!size)
		return false;

	std::vector<unsigned char> buffer;

	// Read contents of the file into the vector
	if (size > 0)
	{
		buffer.resize((size_t)size);
		file.read((char*)(&buffer[0]), size);
	}
	else
		buffer.clear();

	std::vector<unsigned char> out_image;

	if (decodePNG(out_image, width, height, buffer.empty() ? 0 : &buffer[0], (unsigned long)buffer.size(), true) != 0)
		return false;

	size_t bufferSize = out_image.size();
	unsigned int originalBytesPerPixel = (unsigned int)bufferSize / (width * height);
	
	// Force 3 channels
	bytes_per_pixel = 3;

	if (originalBytesPerPixel == 3) {
		pixels = new Color[bufferSize];
		memcpy(pixels, &out_image[0], bufferSize);
	}
	else if (originalBytesPerPixel == 4) {

		unsigned int newBufferSize = width * height * bytes_per_pixel;
		pixels = new Color[newBufferSize];

		unsigned int k = 0;
		for (unsigned int i = 0; i < bufferSize; i += originalBytesPerPixel) {
			pixels[k] = Color(out_image[i], out_image[i + 1], out_image[i + 2]);
			k++;
		}
	}

	// Flip pixels in Y
	if (flip_y)
		FlipY();

	return true;
}

// Loads an image from a TGA file
bool Image::LoadTGA(const char* filename, bool flip_y)
{
	unsigned char TGAheader[12] = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	unsigned char TGAcompare[12];
	unsigned char header[6];
	unsigned int imageSize;
	unsigned int bytesPerPixel;

    std::string sfullPath = absResPath( filename );

	FILE * file = fopen( sfullPath.c_str(), "rb");
   	if ( file == NULL || fread(TGAcompare, 1, sizeof(TGAcompare), file) != sizeof(TGAcompare) ||
		memcmp(TGAheader, TGAcompare, sizeof(TGAheader)) != 0 ||
		fread(header, 1, sizeof(header), file) != sizeof(header))
	{
		std::cerr << "File not found: " << sfullPath.c_str() << std::endl;
		if (file == NULL)
			return NULL;
		else
		{
			fclose(file);
			return NULL;
		}
	}

	TGAInfo* tgainfo = new TGAInfo;
    
	tgainfo->width = header[1] * 256 + header[0];
	tgainfo->height = header[3] * 256 + header[2];
    
	if (tgainfo->width <= 0 || tgainfo->height <= 0 || (header[4] != 24 && header[4] != 32))
	{
		fclose(file);
		delete tgainfo;
		return NULL;
	}
    
	tgainfo->bpp = header[4];
	bytesPerPixel = tgainfo->bpp / 8;
	imageSize = tgainfo->width * tgainfo->height * bytesPerPixel;
    
	tgainfo->data = new unsigned char[imageSize];
    
	if (tgainfo->data == NULL || fread(tgainfo->data, 1, imageSize, file) != imageSize)
	{
		if (tgainfo->data != NULL)
			delete tgainfo->data;
            
		fclose(file);
		delete tgainfo;
		return false;
	}

	fclose(file);

	// Save info in image
	if(pixels)
		delete pixels;

	width = tgainfo->width;
	height = tgainfo->height;
	pixels = new Color[width*height];

	// Convert to float all pixels
	for (unsigned int y = 0; y < height; ++y) {
		for (unsigned int x = 0; x < width; ++x) {
			unsigned int pos = y * width * bytesPerPixel + x * bytesPerPixel;
			// Make sure we don't access out of memory
			if( (pos < imageSize) && (pos + 1 < imageSize) && (pos + 2 < imageSize))
				SetPixel(x, height - y - 1, Color(tgainfo->data[pos + 2], tgainfo->data[pos + 1], tgainfo->data[pos]));
		}
	}

	// Flip pixels in Y
	if (flip_y)
		FlipY();

	delete tgainfo->data;
	delete tgainfo;

	return true;
}

// Saves the image to a TGA file
bool Image::SaveTGA(const char* filename)
{
	unsigned char TGAheader[12] = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	std::string fullPath = absResPath(filename);
	std::cout << "Saving image to: " << fullPath << std::endl;

	FILE *file = fopen(fullPath.c_str(), "wb");
	if ( file == NULL )
	{
		perror("Failed to open file: ");
		return false;
	}

	unsigned short header_short[3];
	header_short[0] = width;
	header_short[1] = height;
	unsigned char* header = (unsigned char*)header_short;
	header[4] = 24;
	header[5] = 0;

	fwrite(TGAheader, 1, sizeof(TGAheader), file);
	fwrite(header, 1, 6, file);

	// Convert pixels to unsigned char
	unsigned char* bytes = new unsigned char[width*height*3];
	for(unsigned int y = 0; y < height; ++y)
		for(unsigned int x = 0; x < width; ++x)
		{
			Color c = pixels[y*width+x];
			unsigned int pos = (y*width+x)*3;
			bytes[pos+2] = c.r;
			bytes[pos+1] = c.g;
			bytes[pos] = c.b;
		}

	fwrite(bytes, 1, width*height*3, file);
	fclose(file);

	return true;
}

void Image::DrawLineDDA(int x0, int y0, int x1, int y1, const Color& c) {

	// Calculate dx, dy and the largest leg of the triangle
	float dx = x1 - x0;
	float dy = y1 - y0;
	float d = std::max(abs(dx), abs(dy));

	// Calculate the direction step vector v
	float vx = dx / d;
	float vy = dy / d;

	// Start drawing the line
	float x = static_cast<float>(x0);
	float y = static_cast<float>(y0);

	for (int i = 0; i <= d; ++i) {
		// Paint pixel at position [floor(x), floor(y)]
		float pixelX = std::floor(x);
		float pixelY = std::floor(y);
		// Ensure the pixel is within the image boundaries
		if (pixelX >= 0 && pixelX < width && pixelY >= 0 && pixelY < height) {
			SetPixel(pixelX, pixelY, c);
		}

		// Increment x and y by v
		x += vx;
		y += vy;
	}

}

void Image::DrawRect(int x, int y, int w, int h, const Color& c)
{
	for (int i = 0; i < w; ++i) {
		SetPixel(x + i, y, c);
		SetPixel(x + i, y + h - 1, c);
	}

	for (int j = 0; j < h; ++j) {
		SetPixel(x, y + j, c);
		SetPixel(x + w - 1, y + j, c);
	}
}

void Image::DrawRectUpdate(int x, int y, int w, int h, const Color& borderColor,
	int borderWidth, bool isFilled, const Color& fillColor) {

	if (borderWidth <= 0) {
		std::cerr << "Error: Border width must be greater than 0." << std::endl;
		return;
	}

	// Draw filled rectangle
	if (isFilled) {
		for (int i = 0; i < w; ++i) {
			for (int j = 0; j < h; ++j) {
				SetPixelSafe(x + i, y + j, fillColor);
			}
		}
	}

	// Draw border
	for (int i = 0; i < borderWidth; ++i) {

		//Draw border horizontal
		for (int j = 0; j < w; ++j) {
			SetPixelSafe(x + j, y - i, borderColor);
			SetPixelSafe(x + j, y + h - 1 + i, borderColor);
		}
		//Draw border vertical
		for (int j = 0; j < h; ++j) {
			SetPixelSafe(x - i, y + j, borderColor);
			SetPixelSafe(x + w - 1 + i, y + j, borderColor);
		}
	}
}

void Image::DrawCircle(int x, int y, int r, const Color& borderColor,
	int borderWidth, bool isFilled, const Color& fillColor) {

	int x1 = r;
	int y1 = 0;
	int v = 1 - x1;

	while (y1 <= x1) {
		if (isFilled) {

			for (int i = x - x1; i <= x + x1; i++) { // revisa les X i ompla de forma vertical
				SetPixelSafe(i, y1 + y, fillColor);
				SetPixelSafe(i, y - y1, fillColor);
			}
			for (int j = x - y1; j <= x + y1; j++) { //Revisa les Y i ompla de forma horitzontal
				SetPixelSafe(j, x1 + y, fillColor);
				SetPixelSafe(j, y - x1, fillColor);
			}
		}
		
		for (int w = 0; w < borderWidth; w++) {
			SetPixelSafe(x + x1 + w, y + y1, borderColor); // Second Octant
			SetPixelSafe(x - x1 - w, y + y1, borderColor); // Fourth Octant
			SetPixelSafe(x + x1 + w, y - y1, borderColor); // Sixth Octant
			SetPixelSafe(x - x1 - w, y - y1, borderColor); // Eighth Octant
			SetPixelSafe(x + y1, y + x1 + w, borderColor); // First Octant
			SetPixelSafe(x - y1, y + x1 + w, borderColor); // Third Octant
			SetPixelSafe(x + y1, y - x1 - w, borderColor); // Fifth Octant
			SetPixelSafe(x - y1, y - x1 - w, borderColor); // Seventh Octant
		}
		
		y1++; //Para mover-se hacia abajo a lo largo del circulo
		if (v <= 0) {
			v += 2 * y1 + 1;
		}
		else {
			x1--;
			v += 2 * (y1 - x1 + 1);

		}
	}
}


void Image::DrawTriangle(const Vector2& p0, const Vector2& p1, const Vector2& p2, const Color& borderColor, int borderWidth, bool isFilled, const Color& fillColor) {
	
	int table_size = height;
	std::vector<Cell> table;
	table.resize(table_size);

	ScanLineDDA(p0.x, p0.y, p1.x, p1.y, table);
	ScanLineDDA(p1.x, p1.y, p2.x, p2.y, table);
	ScanLineDDA(p2.x, p2.y, p0.x, p0.y, table);

	

	// Fill the triangle using a different color
	if (isFilled) {
		DrawLineDDA(p0.x, p0.y, p1.x, p1.y, borderColor);
		DrawLineDDA(p1.x, p1.y, p2.x, p2.y, borderColor);
		DrawLineDDA(p2.x, p2.y, p0.x, p0.y, borderColor);

		for (int y = 0; y < height; ++y) {
			if (table[y].minx <= table[y].maxx) {
				for (int x = table[y].minx; x <= table[y].maxx; ++x) {

						SetPixelSafe(x, y, fillColor);
					
				}
			}
		}
	}


	// Draw the border of the triangle
	for (int i = 0; i < borderWidth; i++) {
		DrawLineDDA(p0.x - i, p0.y - i, p1.x - i, p1.y - i, borderColor);
		DrawLineDDA(p1.x - i, p1.y - i, p2.x - i, p2.y - i, borderColor);
		DrawLineDDA(p2.x - i, p2.y - i, p0.x - i, p0.y - i, borderColor);
	}
}


void Image::ScanLineDDA(int x0, int y0, int x1, int y1,
	std::vector<Cell>& table) {

	// Calculate dx, dy and the largest leg of the triangle
	float dx = x1 - x0;
	float dy = y1 - y0;
	float d = std::max(std::abs(dx), std::abs(dy));

	// Calculate the direction step vector v
	float vx = dx / d;
	float vy = dy / d;

	// Start drawing the line
	float x = static_cast<float>(x0);
	float y = static_cast<float>(y0);

	for (int i = 0; i <= d; ++i) {
		// Update min and max X values in the table
		int currentY = static_cast<int>(y);
		table[currentY].minx = std::min(table[currentY].minx, static_cast<int>(std::floor(x)));
		table[currentY].maxx = std::max(table[currentY].maxx, static_cast<int>(std::floor(x)));

		// Increment x and y by v
		x += vx;
		y += vy;
	}
}


void Image:: DrawImage(const Image& image, int x, int y, bool top) {
	for (int i = 0; i < image.width; ++i) {
		for (int j = 0; j < image.height; ++j) {
			if (top) {
				SetPixelSafe(x+i, y-j, image.GetPixelSafe(i, j));
			}

			else {
				SetPixelSafe(x+i, y + j, image.GetPixelSafe(i, j));
			}
			
		}
	}
			
}


Button::Button(const char* imagePath, int x, int y) {
	bool success = image.LoadPNG(imagePath, false);
	if (!success) {
		std::cerr << "Error loading button image!" << std::endl;
	}

	// Set the position of the button
	std::cerr << "loading correct" << std::endl;
	position = Vector2(x, y);
}

bool Button::IsMouseInside(const Vector2& mousePosition) {

	if (mousePosition.x >= position.x && mousePosition.x <= (position.x + image.width) &&
		-mousePosition.y >=position.y && -mousePosition.y <= (position.y+image.height)) {
		
		std::cerr << "true" << std::endl;
		return true;
	}

	else {
		std::cerr << "false" << std::endl;
		return false;
	}
}




void ParticleSystem::Init(int wi, int he){ // Initialize particles to random positions
	std::srand(static_cast<unsigned>(std::time(nullptr)));
	for (int i = 0; i < MAX_PARTICLES; ++i) {
		particles[i].position.x = static_cast<float>(std::rand() % wi+1);
		particles[i].position.y = static_cast<float>(std::rand() % he+1);
		particles[i].velocity.x = static_cast<float>(std::rand() % 100) / 100.0f - 0.5f; // Random velocity between -0.5 and 0.5
		particles[i].velocity.y = static_cast<float>(std::rand() % 100) / 100.0f - 0.5f;
		particles[i].color = Color::WHITE;
		particles[i].acceleration = 50.0f;
		particles[i].ttl = static_cast<float>(std::rand() % 500) / 100.0f + 1.0f; // Random time to live between 1 and 6 seconds
		particles[i].inactive = false;
	
		
	}
}

void ParticleSystem:: Render(Image* framebuffer) {
	for (int i = 0; i < MAX_PARTICLES; ++i) {
		if (!particles[i].inactive) {
			int x = static_cast<int>(particles[i].position.x);
			int y = static_cast<int>(particles[i].position.y);

			if (x >= 0 && x < 800 && y >= 0 && y < 600) {
				framebuffer->SetPixelSafe(x, y, particles[i].color);
			}
		}
	}
}


void ParticleSystem::Update(float dt) {
	for (int i = 0; i < MAX_PARTICLES; ++i) {
		if (!particles[i].inactive) {


			// Update particle position based on velocity
			particles[i].position.x += particles[i].velocity.x * dt;
			particles[i].position.y += particles[i].velocity.y * dt;

		


			// Update particle velocity based on acceleration
			particles[i].velocity.x -= particles[i].acceleration  * dt;
			particles[i].velocity.y -= particles[i].acceleration * dt;
			
			

			// Decrease time to live
			particles[i].ttl -= dt;

			//Check if the particle's time to live has expired
			if (particles[i].ttl <= 0) {
				particles[i].inactive = true;
			}
		}
	}
}

#ifndef IGNORE_LAMBDAS

// You can apply and algorithm for two images and store the result in the first one
// ForEachPixel( img, img2, [](Color a, Color b) { return a + b; } );
template <typename F>
void ForEachPixel(Image& img, const Image& img2, F f) {
	for(unsigned int pos = 0; pos < img.width * img.height; ++pos)
		img.pixels[pos] = f( img.pixels[pos], img2.pixels[pos] );
}

#endif

FloatImage::FloatImage(unsigned int width, unsigned int height)
{
	this->width = width;
	this->height = height;
	pixels = new float[width * height];
	memset(pixels, 0, width * height * sizeof(float));
}

// Copy constructor
FloatImage::FloatImage(const FloatImage& c) {
	pixels = NULL;

	width = c.width;
	height = c.height;
	if (c.pixels)
	{
		pixels = new float[width * height];
		memcpy(pixels, c.pixels, width * height * sizeof(float));
	}
}

// Assign operator
FloatImage& FloatImage::operator = (const FloatImage& c)
{
	if (pixels) delete pixels;
	pixels = NULL;

	width = c.width;
	height = c.height;
	if (c.pixels)
	{
		pixels = new float[width * height * sizeof(float)];
		memcpy(pixels, c.pixels, width * height * sizeof(float));
	}
	return *this;
}

FloatImage::~FloatImage()
{
	if (pixels)
		delete pixels;
}

// Change image size (the old one will remain in the top-left corner)
void FloatImage::Resize(unsigned int width, unsigned int height)
{
	float* new_pixels = new float[width * height];
	unsigned int min_width = this->width > width ? width : this->width;
	unsigned int min_height = this->height > height ? height : this->height;

	for (unsigned int x = 0; x < min_width; ++x)
		for (unsigned int y = 0; y < min_height; ++y)
			new_pixels[y * width + x] = GetPixel(x, y);

	delete pixels;
	this->width = width;
	this->height = height;
	pixels = new_pixels;
}