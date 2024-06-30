/* */

#include "mesh.h"

//#include "thinker.h"
//#include "statue1.h"
//#include "axe.h"
//#include "greatsword.h"
//#include "dragon_skull.h"
//#include "shark.h"
//#include "cat.h"
//#include "ball.h"
//#include "dog.h"
#include "lol.h"
//#include "skull.h"
//#include "bicycle1low.h"

//#include "perfmon.h"

/*
TFT_MOSI 23 //"SDA"
TFT_SCLK 18
TFT_CS   15  
TFT_DC   2
TFT_RST  4
*/

#define USE_DMA
#define COLOR_DEPTH 16

TFT_eSPI _tft; // TFT_eSPI

//TFT_eSprite _tftdb = TFT_eSprite(&_tft);

TFT_eSprite spr[2] = {TFT_eSprite(&_tft), TFT_eSprite(&_tft) };

bool sprSel = 0;

uint16_t* sprPtr[2];

//Mesh model(thinker::vertexCount, thinker::vertices, 0, 0, thinker::triangleCount, thinker::triangles, thinker::triangleNormals);
//Mesh model(statue1::vertexCount, statue1::vertices, 0, 0, statue1::triangleCount, statue1::triangles, statue1::triangleNormals);
//Mesh model(axe::vertexCount, axe::vertices, 0, 0, axe::triangleCount, axe::triangles, axe::triangleNormals);
//Mesh model(greatsword::vertexCount, greatsword::vertices, 0, 0, greatsword::triangleCount, greatsword::triangles, greatsword::triangleNormals);
//Mesh model(dragon_skull::vertexCount, dragon_skull::vertices, 0, 0, dragon_skull::triangleCount, dragon_skull::triangles, dragon_skull::triangleNormals);
//Mesh model(shark::vertexCount, shark::vertices, 0, 0, shark::triangleCount, shark::triangles, shark::triangleNormals);
//Mesh model(cat::vertexCount, cat::vertices, 0, 0, cat::triangleCount, cat::triangles, cat::triangleNormals);
//Mesh model(ball::vertexCount, ball::vertices, 0, 0, ball::triangleCount, ball::triangles, ball::triangleNormals);
//Mesh model(dog::vertexCount, dog::vertices, 0, 0, dog::triangleCount, dog::triangles, dog::triangleNormals);
Mesh model(lol::vertexCount, lol::vertices, 0, 0, lol::triangleCount, lol::triangles, lol::triangleNormals);
//Mesh model(skull::vertexCount, skull::vertices, 0, 0, skull::triangleCount, skull::triangles, skull::triangleNormals);
//Mesh model(bicycle1low::vertexCount, bicycle1low::vertices, 0, 0, bicycle1low::triangleCount, bicycle1low::triangles, bicycle1low::triangleNormals);

Engine3D engine(1337);

#define TFT_W	128
#define TFT_H	160

static inline uint32_t rgb_to_565(uint32_t r, uint32_t g, uint32_t b)
{
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

// Шейдер для треугольников. Используется указатель на эту функцию в 'model'.
static inline uint32_t triangleShader(int trinangleNo, short *v0, short *v1, short *v2, const signed char *normal, uint32_t color)
{
	const uint32_t nx = 200 + (normal[0] * 255) / 960;
	const uint32_t ny = 200 + (normal[1] * 255) / 960;
	const uint32_t nz = 200 + (normal[2] * 255) / 960;
	return rgb_to_565(nx, nz, ny);
}

void print_free_mem() {
	multi_heap_info_t mhi;
	heap_caps_get_info(&mhi, MALLOC_CAP_DEFAULT);
  	spr[sprSel].print(String("Free") + heap_caps_get_free_size(MALLOC_CAP_DEFAULT) + "B");
}

float rotation1 = 0;
bool rotation1inc = true;

//initial setup
void setup()
{
	Serial.begin(115200);
	_tft.init();
	spr[0].setColorDepth(COLOR_DEPTH);
  	spr[1].setColorDepth(COLOR_DEPTH);
  	sprPtr[0] = (uint16_t*)spr[0].createSprite(TFT_W, TFT_H);
  	sprPtr[1] = (uint16_t*)spr[1].createSprite(TFT_W, TFT_H);
	//spr[sprSel].createSprite(TFT_W, TFT_H);
	//spr[sprSel].fillSprite(32540);
	//spr[sprSel].pushSprite(0, 0);
	//spr[sprSel].setTextColor(rgb_to_565(255, 255, 0));

#ifdef USE_DMA
  	_tft.initDMA(false);
#endif

	Serial.print("\nsetup\n");
	//perfmon_start();
}

void drawModel()
{
	// Перспектива
	static Matrix perspective = Matrix::translation(TFT_W / 2, TFT_H / 2, 0) * Matrix::scaling(100, 100, 100) * Matrix::perspective(90, 1, 10);
	static float u = 0;
	u += 0.005f; // Скорость
	// Поворот
	Matrix rotation = Matrix::rotation(-rotation1, 1, 0, 0) * Matrix::rotation(u, 0, 0, 1);
	Matrix m0 = perspective * Matrix::translation((5 - rotation1) / 2, 0, 7) * rotation * Matrix::scaling(7 * (rotation1 / 4) + 3);
	//Matrix m0 = perspective * Matrix::translation(0, 0, 7) * rotation * Matrix::scaling(9 + rotation1 * 1.5);
	// Преобразование вершин и рёбер
	model.transform(m0, rotation);
	engine.begin();
	model.drawTriangles(engine, 0, triangleShader);
	engine.end(spr[sprSel]);
}

void loop()
{
	static int lastMillis = 0;
	int t = millis();
	static float oldFps = 0;
	float fps = oldFps * 0.9f + 100.f / (t - lastMillis);
	oldFps = fps;
	lastMillis = t;
	spr[sprSel].fillSprite(0);
	drawModel();
	spr[sprSel].setCursor(1, 1);
	spr[sprSel].print(String("fps:") + fps + " t/s:" + int(fps * model.triangleCount));
	//spr[sprSel].setCursor(2, 12);
	//print_free_mem();
	
	_tft.startWrite();
#ifdef USE_DMA
    _tft.pushImageDMA(0, 0, TFT_W, TFT_H, sprPtr[sprSel]);
    sprSel = !sprSel;
#else
	spr[sprSel].pushSprite(0, 0);
#endif
	//_tft.endWrite();

	if (rotation1inc) {
		rotation1 += 0.005f;
		if (rotation1 > 5)
			rotation1inc = false;
	} else {
		rotation1 -= 0.005f;
		if (rotation1 < 0)
			rotation1inc = true;
	}
}

