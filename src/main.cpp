/* */

#include <stdlib.h>
#include "mesh.h"
#include <TFT_eSPI.h>

//#include "thinker.h"
//#include "statue1.h"
//#include "axe.h"
//#include "greatsword.h"
//#include "dragon_skull.h"
//#include "shark.h"
#include "cat.h"
//#include "ball.h"
//#include "dog.h"
//#include "lol.h"
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

#define INPUT_PIN_BUTTON_1 25
#define INPUT_PIN_A 34
#define INPUT_PIN_B 35
#define ADC_RESOLUTION 12
#define ADC_MIN 0
#define ADC_MAX 4096

static uint16_t inputValueA = 0;
static uint16_t inputValueB = 0;
static uint16_t inputCalibrationA = 0;
static uint16_t inputCalibrationB = 0;
static float accelerationA = 0.0;
static float accelerationB = 0.0;
static float translationA = 6.0;
static float translationB = 0.0;
static float translationC = 0.0;
static float rotationA = 0.0;
static float rotationB = 0.0;

static int controlMode = 0;
static int renderMode = 0;
static int stateButton1 = 2;
static int prevStateButton1 = 0;
static bool pressedButton1 = false;
static int64_t timerMsButton1 = 0;

#define USE_DMA
#define COLOR_DEPTH 16
#define TFT_W 128
#define TFT_H 160
TFT_eSPI _tft; // TFT_eSPI
//TFT_eSprite _tftdb = TFT_eSprite(&_tft);
TFT_eSprite spr[2] = {TFT_eSprite(&_tft), TFT_eSprite(&_tft) };
bool sprSel = 0;
uint16_t* sprPtr[2];

static const int initialTrinagleBufferSize = 1337;

//Mesh model(thinker::vertexCount, thinker::vertices, 0, 0, thinker::triangleCount, thinker::triangles, thinker::triangleNormals);
//Mesh model(statue1::vertexCount, statue1::vertices, 0, 0, statue1::triangleCount, statue1::triangles, statue1::triangleNormals);
//Mesh model(axe::vertexCount, axe::vertices, 0, 0, axe::triangleCount, axe::triangles, axe::triangleNormals);
//Mesh model(greatsword::vertexCount, greatsword::vertices, 0, 0, greatsword::triangleCount, greatsword::triangles, greatsword::triangleNormals);
//Mesh model(dragon_skull::vertexCount, dragon_skull::vertices, 0, 0, dragon_skull::triangleCount, dragon_skull::triangles, dragon_skull::triangleNormals);
//Mesh model(shark::vertexCount, shark::vertices, 0, 0, shark::triangleCount, shark::triangles, shark::triangleNormals);
Mesh model(cat::vertexCount, cat::vertices, cat::edgeCount, cat::edges, cat::triangleCount, cat::triangles, cat::triangleNormals);
//Mesh model(ball::vertexCount, ball::vertices, 0, 0, ball::triangleCount, ball::triangles, ball::triangleNormals);
//Mesh model(dog::vertexCount, dog::vertices, 0, 0, dog::triangleCount, dog::triangles, dog::triangleNormals);
//Mesh model(lol::vertexCount, lol::vertices, 0, 0, lol::triangleCount, lol::triangles, lol::triangleNormals);
//Mesh model(skull::vertexCount, skull::vertices, 0, 0, skull::triangleCount, skull::triangles, skull::triangleNormals);
//Mesh model(bicycle1low::vertexCount, bicycle1low::vertices, 0, 0, bicycle1low::triangleCount, bicycle1low::triangles, bicycle1low::triangleNormals);

Engine3D engine(initialTrinagleBufferSize);

static inline uint32_t rgb_to_565(uint32_t r, uint32_t g, uint32_t b)
{
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

// Шейдер для треугольников. Используется указатель на эту функцию в 'model'.
static inline uint32_t triangleShader(int trinangleNo, short *v0, short *v1, short *v2, const signed char *normal, uint32_t color)
{
	const uint32_t nx = min(192 + (normal[0] * 255) / 960, 255);
	const uint32_t ny = min(192 + (normal[1] * 255) / 960, 255);
	const uint32_t nz = min(192 + (normal[2] * 255) / 960, 255);
	return rgb_to_565(nx, nz, ny);
}

// static void print_free_mem() {
// 	multi_heap_info_t mhi;
// 	heap_caps_get_info(&mhi, MALLOC_CAP_DEFAULT);
//   	spr[sprSel].print(heap_caps_get_free_size(MALLOC_CAP_DEFAULT));
// }

// Положение джойстика с учётом калибровки по середине
float adcValToAccel(uint16_t v, uint16_t cal)
{
	float accel = 0;
	if (abs(v - cal) < 50) {
		return 0;
	}
	if (v <= cal) {
		accel = -float((cal - v)) / 32768.0;
	} else {
		accel = float((v - cal)) / 32768.0;
	}
	return accel;
}

static void drawModel()
{
	// Управление джойстиком
	accelerationA = (accelerationA + adcValToAccel(inputValueA, inputCalibrationA)) / 2;
	accelerationB = (accelerationB + adcValToAccel(inputValueB, inputCalibrationB)) / 2;
	if (controlMode == 0) {
		rotationA += accelerationA;
		rotationB += accelerationB;
	} else if (controlMode == 1) {
		translationA = fmax(fmin(translationA + accelerationA, 24.0), 3.5);
	} else if (controlMode == 2) {
		translationC = fmax(fmin(translationC + accelerationA, 16.0), -16.0);
		translationB = fmax(fmin(translationB - accelerationB, 16.0), -16.0);
	}
	// Инициализация единичной матрицы
	// Перспектива 1:10, угол обзора 90°
	static Matrix perspective = Matrix::translation(TFT_W / 2, TFT_H / 2, 0) * Matrix::scaling(100.0, 100.0, 100.0) * Matrix::perspective(90.0, 1.0, 10.0);
	// Поворот
	Matrix rotation = Matrix::rotation(-rotationB, 0, 1.0, 0) * Matrix::rotation(rotationA, 1.0, 0, 0);
	// Перспектива * перемещение * вращение * масштабирование
	Matrix m0 = perspective * Matrix::translation(translationB, translationC, translationA) * rotation * Matrix::scaling(6.0);
	// Преобразование вершин, рёбер и нормалий в соответствии с матрицей
	model.transform(m0, rotation);
	// Отрисовка модели
	engine.begin();

	switch (renderMode) {
		case 0: {
			model.drawTriangles(engine, 0, triangleShader);
			break;
		} 
		case 1: {
			model.drawEdges(spr[sprSel], 0xffff);
			break;
		}
		case 2: {
			model.drawVertices(spr[sprSel], 0xffff);
			break;
		}
		default: {}
	}
	engine.end(spr[sprSel]);
}

void inputTask(void *pvParameter)
{
    while(1)
	{
		// Джойстик
        inputValueA = analogRead(INPUT_PIN_A);
		inputValueB = analogRead(INPUT_PIN_B);
		if (inputCalibrationA == 0) // Калибровка при запуске
		{
			inputCalibrationA = inputValueA;
			inputCalibrationB = inputValueB;
		}

		// Кнопка

		stateButton1 = !digitalRead(INPUT_PIN_BUTTON_1);
		if (stateButton1 != prevStateButton1) {
			if (stateButton1 == 1) {
				controlMode = controlMode == 2 ? 0 : controlMode + 1;
				pressedButton1 = true;
				timerMsButton1 = esp_timer_get_time();
			} else {
				pressedButton1 = false;
			}
			prevStateButton1 = stateButton1;
		}
		
		if (pressedButton1 == true && esp_timer_get_time() - timerMsButton1 >= 1000000) {
			renderMode = renderMode == 2 ? 0 : renderMode + 1;
			pressedButton1 = false;
		}

        vTaskDelay(pdMS_TO_TICKS(100)); // Задача неактивна 100 Мс
    }
}

//initial setup
void setup()
{
	analogReadResolution(ADC_RESOLUTION);
	adcAttachPin(INPUT_PIN_A);
	adcAttachPin(INPUT_PIN_B);

	pinMode(INPUT_PIN_BUTTON_1, INPUT_PULLUP);
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

	printf("\ninited\n");
	
	
	xTaskCreate(&inputTask, "adc_task", 2048, NULL, 5, NULL);
	//perfmon_start();
	while(1)
	{
		static int lastMicros = 0;
		int t = esp_timer_get_time();
		static float oldFps = 0;
		float fps = oldFps * 0.9f + 100000.f / (t - lastMicros);
		oldFps = fps;
		lastMicros = t;

		spr[sprSel].fillSprite(0);
		drawModel();

		spr[sprSel].setCursor(0, 0);
		spr[sprSel].setTextColor(TFT_BLACK, TFT_GREEN);
		spr[sprSel].print(controlMode);
		spr[sprSel].setTextColor(TFT_GREEN);
		spr[sprSel].print(" fps:");
		spr[sprSel].print(int(fps));
		spr[sprSel].print(" t/s:");
		spr[sprSel].print(int(fps * model.triangleCount));

		//print_free_mem();
		
		_tft.startWrite();
#ifdef USE_DMA
		_tft.pushImageDMA(0, 0, TFT_W, TFT_H, sprPtr[sprSel]);
		sprSel = !sprSel;
#else
		spr[sprSel].pushSprite(0, 0);
#endif
	}
}

void loop(){}

