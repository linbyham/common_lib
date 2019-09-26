
#include <stdio.h>

// 从文件读取PPM图片
void ppm_load(char* filename, unsigned char* out_data, int* w, int* h)
{
    char header[1024];
    FILE* fp = NULL;
    int line = 0;

    fp = fopen(filename, "rb");

    // 读取图片格式(例如:"P6")
    // 高宽在第二行非注释数据
    while(line < 2){    
        fgets(header, 1024, fp);
        if(header[0] != '#'){
            ++line;
        }
    }
    // 读取宽高
    sscanf(header,"%d %d\n", w, h);

    // 获取最大像素值
    fgets(header, 20, fp);

    // get rgb data
    fread(out_data, (*w)*(*h)*3, 1, fp);

    fclose(fp);
}

// 写ppm图像到文件
void ppm_save(char* filename, unsigned char* data, int w, int h)
{
    FILE* fp;
    char header[20];

    fp = fopen(filename, "wb");

    // 写图片格式、宽高、最大像素值
    fprintf(fp,"P6\n%d %d\n255\n",w,h);

    // 写RGB数据
    fwrite(data, w*h*3, 1, fp);

    fclose(fp);
}

void yuv_save(char* filename, unsigned char* data, int w, int h)
{
    FILE* fp;
    char header[20];

    fp = fopen(filename, "wb+");

    fwrite(data, (w*h*3)/2, 1, fp);

    fclose(fp);
}

int BGRToRGB(unsigned char *bgr, unsigned char *rgb, int w, int h)
{
	int i = 0;

	if ((bgr == NULL) || (rgb == NULL)) 
	{
		return -1;
	}

	for(i = 0; i< (w * h); i++)    
	{        
		rgb[i*3 + 0] = bgr[i*3 + 2]; 
		rgb[i*3 + 1] = bgr[i*3 + 1]; 
		rgb[i*3 + 2] = bgr[i*3 + 0]; 
	}   
	return 0;
}

void rgb_to_yuv_sw(unsigned char *rgb,
                   unsigned char *yuv,
                   int width,
                   int height)
{
	int frameSize = width * height;
	int chromaSize = frameSize / 4;
 
	int yIndex = 0;
	int uIndex = frameSize;
	int vIndex = frameSize + chromaSize;

    int r, g, b, y, u, v;

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			r = rgb[(i * width + j) * 3 + 0];
			g = rgb[(i * width + j) * 3 + 1];
			b = rgb[(i * width + j) * 3 + 2];
 
			//RGB to YUV
			y = ((66 * r + 129 * g + 25 * b + 128) >> 8) + 16;
			u = ((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128;
			v = ((112 * r - 94 * g - 18 * b + 128) >> 8) + 128;
 
			yuv[yIndex++] = (unsigned char)((y < 0) ? 0 : ((y > 255) ? 255 : y));
			if (i % 2 == 0 && j % 2 == 0)
			{
				yuv[uIndex++] = (unsigned char)((u < 0) ? 0 : ((u > 255) ? 255 : u));
				yuv[vIndex++] = (unsigned char)((v < 0) ? 0 : ((v > 255) ? 255 : v));
			}
		}
    }
}


int main()
{

	int w = 1920;
	int h = 1080;
	int pixel = w*h;
	int rgb_size = pixel*3;
	int yuv_size = w*h*3/2;

	unsigned char *rgb = NULL;
	rgb = malloc(rgb_size);
	if (NULL == rgb)
	{
		printf("malloc rgb buffer error.\r\n");
		return -1;
	}	
	
	unsigned char *yuv = NULL;
	yuv = malloc(yuv_size);
	if (NULL == yuv)
	{
		printf("malloc yuv buffer error.\r\n");
		return -1;
	}

	memset(rgb, 0, rgb_size);
	memset(yuv, 0, yuv_size);

	int w_out, h_out;
	ppm_load("1.ppm", rgb, &w_out, &h_out);
	printf("w_out %d h_out %d\r\n", w_out, h_out);

	printf("hello world\r\n");
	rgb_to_yuv_sw(rgb, yuv, w, h);

	yuv_save("100.yuv", yuv, w, h);


	free(rgb);
	free(yuv);
	return 0;
}
