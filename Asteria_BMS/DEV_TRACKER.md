
#                           BMS Firmware Development Tracker
			
##  Current Version
    1.0.1
			
##  About this file
    	
  This file is meant to track the bug fixes and enhancements done in the BMS code. Each time a pull request is issued, 
  the firmware version is update. 
  
  The BMS code repository has the following branches:
    1. Master:-Major Code Release which has been thoroughly tested both on bench and in flight.
    2. Develop:-This branch contains the sub-releases. All the code committed to this branch is first tested on the bench in a 
	   simulation environment, and then tested in the flight. 
    3. Test_develop:- This branch contains the beta releases. Code committed to this branch have only been tested with minimal
       system (Basic Testing).
  
  The firmware version has the following format:
  <Major Code Release>.<Current Stable Code Release>.<Beta Code Release>
  
  Major Code Release: This number is updated every time the code from the develop is Merged to the Master branch.
  Current Stable Code Release: This number is updated every time the develop branch is merged with the test_develop branch.
  Beta Code Release: This number is update whenever the code in the test_develop branch is update.
  
##  How to maintain this file
    
  This file is needs to be updated whenever any of the branch of the BMS code is updated. 
  
  If a developer is issuing a pull request against test_develop, the <Beta Code Release> parameter must be updated by the developer 
  in the code as well as at the top of this file.
  
  If a developer is merging the test_develop code with the develop, the <Current Stable Code Release> parameter will be updated in 
  the code as well as at the top of the file. This must be done in the merge pull request itself.
  
  If a developer is merging the develop branch code with the Master branch, The <Major Code Release> parameter will be updated in the 
  code as well as at the top of the file. This must be done in the merge pull request itself.
  
  Format of updating the file:
    *** BMS_CODE  1.0.0 ***   (This line should be sub-heading using the ### in the beginning of the file)
	
    Files modified:
    Files Added:
    Files deleted:
  
    Issues Resolved:
	**********************
	
### ****BMS_CODE  1.0.1  ****
	Pull request number: 5
			
	Major Development on application side:
		1. Added logic for creating the log summary file which as of now consist of Total_Number_Of_Files and Power_Up_Number. This is to 
		   create the new log file at every power up as code is under development phase and we want to record the voltage and current reading
		   to calculate the pack capacity. We also want to see the current readings for different current gain settings and sense resistor values
		2. Added/updated the comments for most of the files which are not mentioned under Files Modified tag 
	
	Files Modified: Main.c, BMS_Data_Log.h,BMS_Data_Log.c
			   	   
		   	
### ****BMS_CODE  1.0.0  ****
	Pull request number: 4
	
	In this pull request version number is not added. Only added comment in Main.c  
	
    Major Developement on API side:
        1. Generalized APIs for SPI,I2C,GPIO,RTC,FLASH,SD_SPI,USART,TIMERS are developed.
		2. All APIs contains the functionality for specific micro controller i.e. STM32L432KC.
		   Developer can use the same APIs for other controllers by adding respective controller's low level drivers 
        3. All APIs are tested for their basic functionality
		4. This project contains one hardware configuration (Hardware_Config.h) file in which all peripherals 
		   with their remap functionality is included
		5. Timers (TIM2,TIM6 and TIM6) - tested for interrupt as well as polling method
		   PWM channels (TIM1- 4 channels, TIM2- 4 channels, TIM15- 2 channels and TIM16- 1 channel) - tested for various PWM outputs
		   I2C buses (I2C1 and I2C3) - tested along with BMS ISL94203 chip for EEPROM write and read functionality
		   SD_SPI functionality - tested for various file operations like create, delete,open existing and write, seek,create directories etc
		   USART(USART1 and USART2) both are tested for read and write functionality
		   FLASH - Micro controllers internal flash tested for storing and retrieving user data (4Kb sectors for stm32l432kc)
		   RTC - RTC functionality is tested for almost 12Hrs and found it correct without any lag/lead in real time.
		   GPIO - tested for different GPIOs required in application like SD_CARD_SELECT,SD_CHIP_SELECT, BOARD_LED etc
	
	Major Development on application side:
		1. Created the functions for reading all cell voltages, pack voltage,pack temperature, pack current flowing into and out of the pack
		   and tested the same
		2. Added sleep and wakeup functionality for BMS IC and STM MCU and tested the same
		3. Added logging structure to log the BMS data for 1Hz
		4. External switch is tested for wakeup and to show the LED patterns for SOC and SOH(SOC and SOH logic is yet to be implemented)
		5. The logic is implemented to increase the write/erase cycles of the STM's internal flash and tested the same  
		

	Folder Structure: 
		Asteria_BMS:
			1. Binaries:
				This folder contains .elf file used for debugging the code
			2. Includes:
				Path of all libraries (Include folder locations for all libraries) required to build the project
			3. API_Includes:
				Contains all the general API header files which can be used by various micro controllers for application development.
				Following APIs are developed for BMS application
				FLASH_API.h - Contains prototypes for flash driver functions and controller specific flash addresses
				GPIO_API.h  - Contains prototypes for gpio driver functions and various enumerations for gpios
				I2C_API.h   - Contains prototypes for i2c driver functions and enums for I2C numbers for specific controller
				RTC_API.h	- Contains prototypes for rtc driver functions and enumerations for weekdays, months
				SPI_API.h   - Contains prototypes for spi driver functions and number of spi buses enums for specific controller
				TIMER_API.h - Contains prototypes for timer driver functions and enums for TIMER numbers along with PWM channels
							  for specific controller
				USART_API.h - Contains prototypes for usart driver functions and enums for USART numbers for specific controller
			4. API_Source:
				Contains all the general API source files developed for various micro controllers
				Each header file mentioned above has source file associated with it and source files contains the functions definitions
				for respective module
			5. Include: 
				Contains the header files for application development using the API library files
			6. Middlewares :
				Contains higher level fatfs library files 
			7. Source:
				Contains the Source files for application development using API library files
			8. startup,CMSIS:
				These files are controller specific and developer must add these files for respective controllers
			9. Debug:
				Contains objects files generated for all source files after building the project 
			10. Project configuration:
				This folder contains the preference file (keyboard shortcuts and other c/c++ preferences), settings file
				(c/c++ settings , include paths for project files)	
			11. Enter_Character.txt:
				This is the blank file containing only new line and carriage return characters to exit from CLI after downloading the program
	
		Github Issues Resolved:
			1. RTC Module #2
			2. ISL94203 Module #1 	 	 
			   		    			
						 
