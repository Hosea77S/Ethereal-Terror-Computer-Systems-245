/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "mySprites.h"
#include "time.h"
#include "stdlib.h"
#include "GameSounds.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define Screen_Start (unsigned char*)0x20020000

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2S_HandleTypeDef hi2s3;
DMA_HandleTypeDef hdma_spi3_tx;

DMA_HandleTypeDef hdma_memtomem_dma2_stream0;
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2S3_Init(void);
/* USER CODE BEGIN PFP */



/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* Structure AKA objects and characters*/
struct vector{
	int x;
	int y;
};

struct Barrier{
	uint8_t state;
	uint8_t exists;
	struct vector pos;
	const uint8_t* state1;
	const uint8_t* state2;
	const uint8_t* state3;
	const uint8_t* state4;
};

struct Invader{
	struct vector pos;
	const uint8_t* sprite1;
	const uint8_t* sprite2;
	uint8_t alive;
	uint16_t points;
};

struct InvaderRow{
	struct vector pos;
	int rightpos;
	struct vector v;
	struct Invader *Invdr;
	uint8_t exists;
	uint8_t animator;
};

struct Player{
	struct vector pos;
	const uint8_t* sprite;
	uint8_t alive;
	uint8_t life;
};

struct UFO{
	struct vector pos;
	const uint8_t* openM;
	const uint8_t* closedM;
	uint8_t alive;
	uint8_t life;
	uint16_t points;
};

struct Projectile{
	struct vector v;
	struct vector pos;
	const uint8_t* sprite;
	const uint8_t* boom;
	uint8_t fire;
};

void updateScrn();
void GameLoop();
void TitleScrn();
void displayint(uint16_t val, uint32_t* screenptr);
void PaintShip(unsigned char* ptr_screen, const unsigned char* ptr_sprite, int x, int y, int sprite_w, int sprite_h);
void DrawBarriers();
void DeathScene(int x, int y);
void clearScrn();

void PaintInvaderRow(int n);
void updateInvadersMatrix();
void PacMan();

void moveInvader();
void DropBomb();
void moveBomb(int select);

void initObjects();

void PaintLivingInvaders(int n,int index, int x, int y);
void shoot();

uint8_t animate = 0;
uint16_t score = 0;
uint16_t highscore = 0;
struct Barrier barrier[3];

volatile uint8_t refresh = 0;
volatile uint8_t shot = 0;
volatile uint8_t shiftdone = 0;
//invader matrix vectors
int Invdrx;
int Invdry;
int Invdr_speedx;
int Invdr_speedy;
int speedboost;
uint8_t numOfInvdrs = 24;


int rows = 4;
int columns = 6;
int col_offset;
int right_col_offset = 0;
int row_offset;

//Game objects
struct Invader Invaders1[6];
struct Invader Invaders2[6];
struct Invader Invaders3[6];
struct Invader Invaders4[6];
struct InvaderRow Row[4];
uint8_t InvaderLives[4][6] = {
		{1,1,1,1,1,1},
		{1,1,1,1,1,1},
		{1,1,1,1,1,1},
		{1,1,1,1,1,1},
};
int rowToMove;


struct Player Ship;
uint8_t hearts[3];

struct Projectile Bullet;
uint32_t Timeshot = 0;
struct Projectile Bombs[2];
int MAX_BOMBS = 2;
int BombsDropped = 0;
uint8_t Drop = 0;


//Boundaries
uint32_t Leftwall;
uint32_t Rightwall;
uint32_t Bottomwall;
uint32_t Topwall;

//Game Time and pacman
uint32_t lastcheck;
uint32_t runtime;
uint32_t lastsight;
uint8_t pacmanReleased;
struct UFO Pacman;
uint8_t play = 0;


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2S3_Init();
  /* USER CODE BEGIN 2 */

  //Walls

  Leftwall = 10;
  Rightwall = 310;
  Bottomwall = 184;
  Topwall = 12;

  //Enemy speed

  Invdr_speedx = 1;
  Invdr_speedy = 1;
  speedboost = 1;

  //initial Invader matrix positions

  Invdrx = Leftwall + 4;
  Invdry = Topwall + 18;

  //Game objects initialized

  initObjects();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if(refresh){
		  //clearScrn();

		  runtime = HAL_GetTick() - lastcheck;
		  refresh = 0;

		  if(Ship.life == 0){
			  Ship.alive = 0;
			  play = 0;
			  clearScrn();
		  }

		  if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) ){
			  shot = 1;
		  }

		  //Title
		  if(!play){
			  TitleScrn();
		  }
		  //Game plays
		  else if(Ship.alive && play){
			  play = 1;
			  clearScrn();
			  GameLoop();


			  if(numOfInvdrs == 0){
				HAL_Delay(100);
				clearScrn();
				Ship.alive = 0;
				play = 0;
				shot = 0;
			  }
		  }

		//check if bottom of screen
		for(int i=0; i<rows; i++){
			for(int j=col_offset; j<(columns-right_col_offset); j++){
				if((Row[i].Invdr[j].pos.y >= 150) && (Row[i].Invdr[j].alive)){
					Ship.alive = 0;
					HAL_Delay(50);
					shot = 0;
					clearScrn();
				}
			}
		}

	  }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Macro to configure the PLL multiplication factor 
  */
  __HAL_RCC_PLL_PLLM_CONFIG(16);
  /** Macro to configure the PLL clock source 
  */
  __HAL_RCC_PLL_PLLSOURCE_CONFIG(RCC_PLLSOURCE_HSI);
  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2S;
  PeriphClkInitStruct.PLLI2S.PLLI2SN = 192;
  PeriphClkInitStruct.PLLI2S.PLLI2SM = 16;
  PeriphClkInitStruct.PLLI2S.PLLI2SR = 2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2S3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2S3_Init(void)
{

  /* USER CODE BEGIN I2S3_Init 0 */

  /* USER CODE END I2S3_Init 0 */

  /* USER CODE BEGIN I2S3_Init 1 */

  /* USER CODE END I2S3_Init 1 */
  hi2s3.Instance = SPI3;
  hi2s3.Init.Mode = I2S_MODE_MASTER_TX;
  hi2s3.Init.Standard = I2S_STANDARD_PHILIPS;
  hi2s3.Init.DataFormat = I2S_DATAFORMAT_16B;
  hi2s3.Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE;
  hi2s3.Init.AudioFreq = I2S_AUDIOFREQ_8K;
  hi2s3.Init.CPOL = I2S_CPOL_LOW;
  hi2s3.Init.ClockSource = I2S_CLOCK_PLL;
  hi2s3.Init.FullDuplexMode = I2S_FULLDUPLEXMODE_DISABLE;
  if (HAL_I2S_Init(&hi2s3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2S3_Init 2 */

  /* USER CODE END I2S3_Init 2 */

}

/** 
  * Enable DMA controller clock
  * Configure DMA for memory to memory transfers
  *   hdma_memtomem_dma2_stream0
  */
static void MX_DMA_Init(void) 
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* Configure DMA request hdma_memtomem_dma2_stream0 on DMA2_Stream0 */
  hdma_memtomem_dma2_stream0.Instance = DMA2_Stream0;
  hdma_memtomem_dma2_stream0.Init.Channel = DMA_CHANNEL_0;
  hdma_memtomem_dma2_stream0.Init.Direction = DMA_MEMORY_TO_MEMORY;
  hdma_memtomem_dma2_stream0.Init.PeriphInc = DMA_PINC_ENABLE;
  hdma_memtomem_dma2_stream0.Init.MemInc = DMA_MINC_ENABLE;
  hdma_memtomem_dma2_stream0.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  hdma_memtomem_dma2_stream0.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
  hdma_memtomem_dma2_stream0.Init.Mode = DMA_NORMAL;
  hdma_memtomem_dma2_stream0.Init.Priority = DMA_PRIORITY_LOW;
  hdma_memtomem_dma2_stream0.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
  hdma_memtomem_dma2_stream0.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
  hdma_memtomem_dma2_stream0.Init.MemBurst = DMA_MBURST_SINGLE;
  hdma_memtomem_dma2_stream0.Init.PeriphBurst = DMA_PBURST_SINGLE;
  if (HAL_DMA_Init(&hdma_memtomem_dma2_stream0) != HAL_OK)
  {
    Error_Handler( );
  }

  /* DMA interrupt init */
  /* DMA1_Stream5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PD8 PD9 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : PB4 */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

}

/* USER CODE BEGIN 4 */

void GameLoop(){

	  updateScrn();

	  //select invader to move [  Row[rowToMove]  ]
	  if(rowToMove<3)
		  rowToMove++;
	  else
		  rowToMove = 0;

	  //select sprites to draw
	  if(animate){
		  animate = 0;
	  }
	  else{
		  animate = 1;
	  }

	  PacMan();

	  //set Ship's new position

	  if ((Ship.pos.x < (Rightwall - 16)) && HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_9))
	 	   Ship.pos.x += 4;

	  if ((Ship.pos.x > Leftwall) && HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_10))
		   Ship.pos.x -= 4;

}

void TitleScrn(){
	PaintShip(Screen_Start, Title, 64, 40, Title_W, Title_H);
	PaintShip(Screen_Start, Start_Game, 35, 120, 160, 12);
	PaintShip(Screen_Start, SUN, 35, 135, 80, 12);
	PaintShip(Screen_Start, HighScoreTitle, 100, 3, 72, 9);
    displayint(highscore, (uint32_t*)(0x20020000 + 320*3 + 190));

	if(score>highscore){
		highscore = score;
	}

	if(shot){
		clearScrn();
		initObjects();
		Ship.alive = 1;
		Ship.life = 3;
		speedboost = 1;
		shot = 0;
		play = 1;
		score = 0;
		lastsight = HAL_GetTick();

	}
}

void displayint(uint16_t val, uint32_t* screenptr)
{
	uint8_t digit = 0;
	uint32_t* digitptr;
	uint32_t* scrcopyptr;
	for (int i = 0; i < 5; i++)
	{
		digit = val % 10;
		val /= 10;

		scrcopyptr = screenptr;
		digitptr = (uint32_t*)(digits + (digit << 3));
		for (int i = 0; i < 9; i++)
		{
			for (int j = 0; j < 2; j++)
			{
				*scrcopyptr++ = *digitptr++;
			}
			digitptr += 18;
			scrcopyptr += 78;

		}
		screenptr -= 2;
	}
}

void PaintShip(unsigned char* ptr_screen, const unsigned char* ptr_sprite, int x, int y, int sprite_w, int sprite_h){

	uint32_t* coord;
	uint32_t* ptrColour = (uint32_t*)ptr_sprite;

	for(int h = 0; h < sprite_h; h++){
		coord = (uint32_t*)(ptr_screen + 320*(y+h) + x);
		for(int w = 0; (w< sprite_w/4); w++){

			*coord++ = *ptrColour++;

		}
	}

}

void clearScrn(){
	uint32_t* ptrscreen = (uint32_t*)0x20020000;
	*ptrscreen = 0;
	shiftdone = 0;
	HAL_DMA_Start_IT(&hdma_memtomem_dma2_stream0, 0x20020000, 0x20020004, 16000);
	while (!shiftdone);
}

void updateScrn(){
	//display score
	PaintShip(Screen_Start, ScoreTitle, 10, 3, 40, 9);
	displayint(score, (uint32_t*)(0x20020000 + 320*3 + 82));

	//draw hearts
	for(int i=0; i<Ship.life; i++){
		PaintShip(Screen_Start, Hearts, 280 + 12*i, 0, 12, 12);
	}

	//Draw rows
	//Update invader matrix dimensions
	if(numOfInvdrs != 0){
		updateInvadersMatrix();
		moveInvader();
	}

	for(int j = 0; j<rows; j++){
		if(Row[j].exists){
			PaintInvaderRow(j);
		}
	}


	//barriers
	DrawBarriers();

	//Pacman
	if(pacmanReleased){
		//select sprite
		if(animate)
			PaintShip(Screen_Start, Pacman.openM, Pacman.pos.x, Pacman.pos.y, 16, 16);
		else
			PaintShip(Screen_Start, Pacman.closedM, Pacman.pos.x, Pacman.pos.y, 16, 16);

		if(Pacman.pos.x < Rightwall - 16){
			Pacman.pos.x += 4;
		}
		else{
			pacmanReleased = 0;
			lastsight = HAL_GetTick();
		}
	}

	//if(Ship.alive)
		PaintShip(Screen_Start, Ship.sprite, Ship.pos.x, Ship.pos.y, Ship_W, Ship_H);

	/***      Player Shoots   ***/
	if(shot && !Bullet.fire){
		//set bulet's position from where it will be fired
		Timeshot = HAL_GetTick();
		Bullet.pos.x = Ship.pos.x + 2;
		Bullet.pos.y = Ship.pos.y - 8;
		Bullet.fire = 1;

		//relax after bullet is shot
		shot = 0;
		HAL_I2S_Transmit_DMA(&hi2s3, (uint16_t*)Photon_Blast,  SHOOTLEN);
	}

	if(Bullet.fire){
		//move bullet
		shoot();
		//paint bullet if fire = 1
		if(Bullet.fire)
			PaintShip(Screen_Start, Bullet.sprite, Bullet.pos.x, Bullet.pos.y, BLT_W, BLT_H);
	}

	/***      Invader Drops Bomb  ***/
	DropBomb();

	for(int i = 0; i< MAX_BOMBS; i++){
	  		if(Bombs[i].fire){
	  			moveBomb(i);

	  			if(Bombs[i].fire){
	  				PaintShip(Screen_Start, Bomb, Bombs[i].pos.x, Bombs[i].pos.y, 4, 4);
	  			}//end if 2

	  		}//end if 1
	}//end for 1

}


 void DrawBarriers(){
	 for(int i = 0; i<3; i++){
		if(barrier[i].exists){
			if(barrier[i].state == 1)
				PaintShip(Screen_Start, barrier[i].state4, barrier[i].pos.x, barrier[i].pos.y, 48, 8);

			else if(barrier[i].state == 2)
				PaintShip(Screen_Start, barrier[i].state3, barrier[i].pos.x, barrier[i].pos.y, 48, 8);

			else if(barrier[i].state == 3)
				PaintShip(Screen_Start, barrier[i].state2, barrier[i].pos.x, barrier[i].pos.y, 48, 8);

			else
				PaintShip(Screen_Start, barrier[i].state1, barrier[i].pos.x, barrier[i].pos.y, 48, 8);

		}
	}
 }

void PaintLivingInvaders(int n, int index, int x, int y){
	if(Row[n].Invdr[index].alive){
		if(Row[n].animator)
			PaintShip(Screen_Start, Row[n].Invdr[index].sprite1, x, y, Ghost1_W, Ghost1_H);
		else
			PaintShip(Screen_Start, Row[n].Invdr[index].sprite2, x, y, Ghost1_W, Ghost1_H);
	}
}
void PaintInvaderRow(int n){
	//draw 5 rows
	for(int i = col_offset; i<(columns-right_col_offset); i++){
		PaintLivingInvaders(n, i, Row[n].Invdr[i].pos.x, Row[n].Invdr[i].pos.y);
	}
}

void moveInvader(){

	//select invader sprite
	if(Row[rowToMove].animator){
		  Row[rowToMove].animator = 0;
    }
    else{
    	Row[rowToMove].animator = 1;
    }

	if((Row[rowToMove].pos.x > Leftwall) && (Row[rowToMove].v.x < 0)){

		//move left
		Row[rowToMove].pos.x += speedboost*Row[rowToMove].v.x;
		Row[rowToMove].rightpos += speedboost*Row[rowToMove].v.x;
		//move each invader
		for(int i=col_offset; i<(columns-right_col_offset); i++)
			Row[rowToMove].Invdr[i].pos.x += speedboost*Row[rowToMove].v.x;


	}

	else if((Row[rowToMove].pos.x <= Leftwall) && (Row[rowToMove].v.x < 0)){

		//move down
		if(Row[rowToMove].pos.y <=(Bottomwall - Ghost1_H)){
			Row[rowToMove].pos.y += speedboost*Row[rowToMove].v.y;
			for(int i=col_offset; i<(columns-right_col_offset); i++)
				Row[rowToMove].Invdr[i].pos.y += speedboost*Row[rowToMove].v.y;
		}
		//else game over
		Row[rowToMove].v.x = 1;

	}

	else if((Row[rowToMove].rightpos < (Rightwall - Ghost1_W )) && (Row[rowToMove].v.x > 0)){

		//move right
		Row[rowToMove].pos.x += speedboost*Row[rowToMove].v.x;
		Row[rowToMove].rightpos += speedboost*Row[rowToMove].v.x;
		//move each invader
		for(int i=col_offset; i<(columns-right_col_offset); i++)
			Row[rowToMove].Invdr[i].pos.x += speedboost*Row[rowToMove].v.x;

	}

	else if((Row[rowToMove].rightpos >= (Rightwall-Ghost1_W) ) && (Row[rowToMove].v.x > 0)){

		//move down
		if(Row[rowToMove].pos.y <=(Bottomwall - Ghost1_H)){
			Row[rowToMove].pos.y += speedboost*Row[rowToMove].v.y;
			for(int i=col_offset; i<(columns-right_col_offset); i++)
				Row[rowToMove].Invdr[i].pos.y += speedboost*Row[rowToMove].v.y;
		//else game over
		Row[rowToMove].v.x = -1;
		}

     }


}


void shoot(){
	int targetx;
	int targety;
	int x = Bullet.pos.x + 4;
	int y = Bullet.pos.y + 4;

	//scan what object (except top wall) is ahead of bullet
	for(int i = row_offset; i<rows; i++){
		for(int j = col_offset; j<columns; j++){

			if(Row[i].Invdr[j].alive && Bullet.fire){
				targetx = Row[i].Invdr[j].pos.x;
				targety = Row[i].Invdr[j].pos.y;

				if(  ((x >= targetx) && (x < (targetx + 16))) && ( (y >= targety) && (y < (targety + 16))  )  ){
					Row[i].Invdr[j].alive = 0;
					numOfInvdrs--;
					score += Row[i].Invdr[j].points;
					if(speedboost<=10 && numOfInvdrs<=15)
						speedboost++;
					//paint explosio :D
					HAL_I2S_Transmit_DMA(&hi2s3, (uint16_t*)Invader_Shot,  INVDR_SHOTLEN);
					PaintShip(Screen_Start, Bullet.boom, targetx, targety, 16, 16);
					Bullet.fire = 0;
				}//end if 2

			}//end if 1
		}
	}

	//if Pacman is there
	if(pacmanReleased && Bullet.fire){
		if(  ((x >= Pacman.pos.x) && (x < (Pacman.pos.x + 16))) && ( (y >= Pacman.pos.y) && (y < (Pacman.pos.y + 16))  )  ){
			score += Pacman.points;
			pacmanReleased = 0;
			HAL_I2S_Transmit_DMA(&hi2s3, (uint16_t*)Invader_Shot,  INVDR_SHOTLEN);
			PaintShip(Screen_Start, Bullet.boom, Pacman.pos.x, Pacman.pos.y, 16, 16);
			Bullet.fire = 0;
			lastsight = HAL_GetTick();
		}

	}

	//hits barrier
	if(Bullet.fire){
		for(int i = 0; i< 3; i++){
			if(barrier[i].exists &&  ( (x >= barrier[i].pos.x) && (x < (barrier[i].pos.x + 48))) && ( (y >= barrier[i].pos.y-8) && (y < (barrier[i].pos.y + 8))  )   ){
				barrier[i].state--;
				Bullet.fire = 0;
				if(!barrier[i].state)
					barrier[i].exists = 0;
			}
		}
	}

	//if nothing is ahead of bullet, keep moving it up
	if(Bullet.fire){
		if(Bullet.pos.y <= Topwall){
			Bullet.fire = 0;
		}
		else {
			Bullet.pos.y -= Bullet.v.y;
		}
	}


}

void DropBomb(){
	//Use a living Invader
	for(int i = 0; i<4 ;i++){
		for(int j = col_offset; j<columns; j++){
			if(Row[i].Invdr[j].alive && (BombsDropped<MAX_BOMBS) ){
				Drop = rand()%100;

				if(Drop<7){
					BombsDropped++;
					Bombs[BombsDropped-1].fire = 1;
					Bombs[BombsDropped-1].pos.x = Row[i].Invdr[j].pos.x + 6;
					Bombs[BombsDropped-1].pos.y = Row[i].Invdr[j].pos.y + Ghost1_H;;
				}// end of 2
			}//end if 1

		}// end for 2
	}//end for 1

}//end function

void moveBomb(int select){
	int targetx = Ship.pos.x;
	int targety = Ship.pos.y;
	int x = Bombs[select].pos.x;
	int y = Bombs[select].pos.y + 4;

	//Scan for player
	if( (x>=targetx) && (x<(targetx+Ship_W)) && (y>=targety) ){
		Bombs[select].fire = 0;
		BombsDropped--;
		Ship.life--;
		DeathScene(targetx, targety);
	}

	//hits barrier
	if(Bombs[select].fire){
		for(int i = 0; i< 3; i++){
			if(barrier[i].exists &&  ( (x >= barrier[i].pos.x) && (x < (barrier[i].pos.x + 48))) && ( (y >= barrier[i].pos.y-2) && (y < (barrier[i].pos.y + 8))  )   ){
				barrier[i].state--;
				Bombs[select].fire = 0;
				BombsDropped--;
				if(!barrier[i].state)
					barrier[i].exists = 0;
			}
		}
	}

	//else if(Bombs[select].fire)
	if(Bombs[select].fire){
		if( (y >= Bottomwall) && Bombs[select].fire){
			Bombs[select].fire = 0;
			BombsDropped--;
		}
		else{
			Bombs[select].pos.y += Bombs[select].v.y;
		}
	}
}

void updateInvadersMatrix(){
	  uint8_t offsetcolumns = 0;
	  uint8_t decrement_columns = 0;

	  //test columns
	  for(int i = 0; i<rows; i++){
		  offsetcolumns += Row[i].Invdr[col_offset].alive;
		  decrement_columns += Row[i].Invdr[columns-1-right_col_offset].alive;
	  }

		if(offsetcolumns == 0){
			col_offset += 1;
			Row[0].pos.x = Row[0].Invdr[col_offset].pos.x;
			Row[1].pos.x = Row[1].Invdr[col_offset].pos.x;
			Row[2].pos.x = Row[2].Invdr[col_offset].pos.x;
			Row[3].pos.x = Row[3].Invdr[col_offset].pos.x;
		}

		if(decrement_columns == 0){
			//removes last column which is dead
			right_col_offset += 1;
			Row[0].rightpos = Row[0].Invdr[columns-1-right_col_offset].pos.x;
			Row[1].rightpos = Row[1].Invdr[columns-1-right_col_offset].pos.x;
			Row[2].rightpos = Row[2].Invdr[columns-1-right_col_offset].pos.x;
			Row[3].rightpos = Row[3].Invdr[columns-1-right_col_offset].pos.x;
		}

}

void PacMan(){
	if((HAL_GetTick() - lastsight) > 2000){
		pacmanReleased = 1;
		Pacman.pos.x = Leftwall;
		lastsight = HAL_GetTick();
	}

	//shuffle pacman points
	int choose = rand()%5;
	if(choose == 0)
		Pacman.points = 20;
	else if(choose == 1)
			Pacman.points = 30;
	else if(choose == 2)
			Pacman.points = 40;
	else if(choose == 3)
			Pacman.points = 80;
	else
			Pacman.points = 150;
}

void DeathScene(int x, int y){
	PaintShip(Screen_Start, Mush_Cloud, x, y, 24, 24);
	HAL_I2S_Transmit_DMA(&hi2s3, (uint16_t*)Ship_explode,  MUSH_CLOUDLEN);
	HAL_Delay(50);
}

void initObjects(){

	srand(time(NULL));
	lastcheck = HAL_GetTick();
	numOfInvdrs = 24;

	row_offset = 0;
    rows = 4;
	columns = 6;
	col_offset = 0;
	right_col_offset = 0;
	//row_offset;

	for(int i = 0; i<3; i++){
		barrier[i].exists = 1;
		barrier[i].state = 7;
		barrier[i].state1 = wall1;
		barrier[i].state2 = wall2;
		barrier[i].state3 = wall3;
		barrier[i].state4 = wall4;
		barrier[i].pos.x = 44 + i*92;
		barrier[i].pos.y = 160;

	}

	Ship.pos.x = 280;
    Ship.pos.y = 180;
    Ship.alive = 0;
    Ship.life = 3;
    Ship.sprite = shipTest;

    Pacman.pos.x = Leftwall;;
    Pacman.pos.y = Topwall;
    Pacman.alive = 0;
    Pacman.openM = UFO1;
    Pacman.closedM = UFO2;
	pacmanReleased = 0;
	Pacman.points = 0;

	Bullet.fire = 0;
	Bullet.pos.x = 0;
	Bullet.pos.y = 0;
	Bullet.sprite = Photon_Pack;
    Bullet.boom = Explosion;
	Bullet.v.x = 0;
	Bullet.v.y = 16;

	BombsDropped = 0;
	Drop = 0;
	for(int i = 0; i<MAX_BOMBS; i++){
		Bombs[i].fire = 0;
		Bombs[i].pos.x = 0;
		Bombs[i].pos.y = 0;
		Bombs[i].sprite = Bomb;
		Bombs[i].boom = Mush_Cloud;
		Bombs[i].v.x = 0;
		Bombs[i].v.y = 8;
	}

	for(int c = 0; c<columns; c++){
			Invaders1[c].pos.x = Invdrx + c*Ghost1_W*2;
			Invaders1[c].pos.y = Invdry + 60;
			Invaders1[c].sprite1 = Ghost1;
			Invaders1[c].sprite2 = Ghost1;
			Invaders1[c].alive = InvaderLives[0][c];
			Invaders1[c].points = 10;
	}
	for(int c = 0; c<columns; c++){
			Invaders2[c].pos.x = Invdrx + c*Ghost1_W*2;
			Invaders2[c].pos.y = Invdry + 40;
			Invaders2[c].sprite1 = Ghost1;
			Invaders2[c].sprite2 = Ghost1;
			Invaders2[c].alive = InvaderLives[1][c];
			Invaders2[c].points = 10;
		}
	for(int c = 0; c<columns; c++){
			Invaders3[c].pos.x =  Invdrx + c*Ghost1_W*2;
			Invaders3[c].pos.y = Invdry + 20;
			Invaders3[c].sprite1 = Ghost2;
			Invaders3[c].sprite2 = Ghost2_2;
			Invaders3[c].alive = InvaderLives[2][c];
			Invaders3[c].points = 20;
		}
	for(int c = 0; c<columns; c++){
			Invaders4[c].pos.x = Invdrx + c*Ghost1_W*2;
			Invaders4[c].pos.y = Invdry;
			Invaders4[c].sprite1 = Ghost3;
			Invaders4[c].sprite2 = Ghost3_2;
			Invaders4[c].alive = InvaderLives[3][c];
			Invaders4[c].points = 30;
		}

	Row[3].Invdr = Invaders4;
	Row[3].exists = 1;
	Row[3].pos.x = Invdrx;
	Row[3].pos.y = Invdry;
	Row[3].rightpos = Row[3].Invdr[5].pos.x;
	Row[3].v.x = 1;
	Row[3].v.y = 1;
	Row[3].animator = 0;

	Row[2].Invdr = Invaders3;
	Row[2].exists = 1;
	Row[2].pos.x = Invdrx;
	Row[2].pos.y = Invdry + 20;
	Row[2].rightpos = Row[2].Invdr[5].pos.x;
	Row[2].v.x = 1;
	Row[2].v.y = 1;
	Row[2].animator = 0;


	Row[1].Invdr = Invaders2;
	Row[1].exists = 1;
	Row[1].pos.x = Invdrx;
	Row[1].pos.y = Invdry + 40;
	Row[1].rightpos = Row[1].Invdr[5].pos.x;
	Row[1].v.x = 1;
	Row[1].v.y = 1;
	Row[1].animator = 0;


	Row[0].Invdr = Invaders1;
	Row[0].exists = 1;
	Row[0].pos.x = Invdrx;
	Row[0].pos.y = Invdry + 60;
	Row[0].rightpos = Row[0].Invdr[5].pos.x;
	Row[0].v.x = 1;
	Row[0].v.y = 1;
	Row[0].animator = 0;


}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
