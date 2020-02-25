/******************************************************************************

  The contents of this file are subject to the Mozilla Public License
  Version 1.1 (the "License"); you may not use this file except in
  compliance with the License. You may obtain a copy of the License at
  http://www.mozilla.org/MPL/

  Software distributed under the License is distributed on an "AS IS"
  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
  the License for the specific language governing rights and limitations
  under the License. 

  The Original Code is DCC Library for the RCX.

  The Initial Developer of the Original Code is Mark Riley. Portions
  created by Mark Riley are Copyright (C) 2003 Mark Riley. All Rights
  Reserved. 

     File : dccdemo.c
  Created : 6/12/2003
  Purpose : DCC demonstration program for DCC library

******************************************************************************/

#include <unistd.h>
#include <dmotor.h>
#include <dsensor.h>
#include <conio.h>
#include <dsound.h>
#include <dcc.h>

#define MASK (MOTOR_A_MASK | MOTOR_B_MASK | MOTOR_C_MASK)

#define MAX_ANGLE 48

#define ADDR 3

#define DCC_FUNC_GRP_0 0x80
#define DCC_FUNC_GRP_1 0xB0
#define DCC_FUNC_GRP_2 0xA0

// state of decoder functions FL through F4
unsigned char funcs [] = { DCC_FUNC_GRP_0, DCC_FUNC_GRP_1, DCC_FUNC_GRP_2 };

void dcc_func(int addr, unsigned char func)
{

  // Determine function group and toggle the specified function flag
  if ((0xE0 & func) == DCC_FUNC_GRP_0)
  {
    funcs[0] = DCC_FUNC_GRP_0 | ((funcs[0] ^ func) & 0x1F);
    p_buffer[1] = funcs[0];
    cputc_0('0');
  }
  else if ((0xF0 & func) == DCC_FUNC_GRP_1)
  {
    funcs[1] = DCC_FUNC_GRP_1 | ((funcs[1] ^ func) & 0x0F);
    p_buffer[1] = funcs[1];
    cputc_0('1');
  }
  else if ((0xF0 & func) == DCC_FUNC_GRP_2)
  {
    funcs[2] = DCC_FUNC_GRP_2 | ((funcs[2] ^ func) & 0x0F);
    p_buffer[1] = funcs[2];
    cputc_0('2');
  }
  else
  {
    cputc_0('-');
  }
  
  // Fill the command buffer
  p_buffer[0] = addr;

  // Send the command
  dcc_ops_packet(p_buffer, 2);
}

void dcc_sleep(int sec)
{
  dcc_msleep(sec * 1000);
}

void dcc_msleep(int msec)
{
  int tmStart = (int)get_system_up_time();
  int i = 0;
  
  while(tmStart + msec >= (int)get_system_up_time())
  {
    p_buffer[0] = ADDR;
    for(i = 0; i < 3; i++)
    {
      dcc_idle();
      p_buffer[1] = funcs[i];
      dcc_ops_packet(p_buffer, 2);
      dcc_idle();
    }
  }
}

void dcc_funcFlip(int bank, int func)
{
  switch (bank)
  {
    case 0:
      dcc_FL_F4(ADDR, func);
      dcc_msleep(20);
      dcc_FL_F4(ADDR, 0);
      break;
    case 1:
      dcc_F5_F8(ADDR, func);
      dcc_msleep(20);
      dcc_F5_F8(ADDR, 0);
      break;
    case 2:
      dcc_F9_F12(ADDR, func);
      dcc_msleep(20);
      dcc_F9_F12(ADDR, 0);
      break;
  }
}


int main()
  {
  unsigned char old_dm_mask = dm_mask;
  int angle = 0;
  char state = 0;
  int time;

  dm_mask &= ~MASK;
  dcc_mask = MASK;

  ds_active(&SENSOR_3);
  ds_rotation_on(&SENSOR_3);
  ds_rotation_set(&SENSOR_3, 0);
  
  sleep(1);
  
  cputs("Pwr");
//  dsound_system(0);
  dcc_power_on();
  dcc_sleep(1);


  cputs("Up");
//  dsound_system(0);
  dcc_func(ADDR, DCC_FUNC_GRP_1 | DCC_F6);
//  dcc_F5_F8(ADDR, DCC_F6);
//  dcc_msleep(250);
//  dcc_F5_F8(ADDR, 0);
  dcc_sleep(5);
  
  
  cputs("Lite");
//  dsound_system(0);
  dcc_func(ADDR, DCC_FUNC_GRP_0 | DCC_FL);
  dcc_sleep(5);
  
  cputs("Bell");
//  dsound_system(0);
  dcc_func(ADDR, DCC_FUNC_GRP_0 | DCC_F1);
//  dcc_func(0, DCC_F1);
//  dcc_FL_F4(ADDR, DCC_F1 + DCC_F2);
  dcc_sleep(5);
	  

  cputs("bOff");
//  dsound_system(0);
  dcc_func(ADDR, DCC_FUNC_GRP_0 | DCC_F1);
//  dcc_FL_F4(ADDR, 0);
//  dcc_func(0, DCC_F1);
//  dcc_FL_F4(ADDR, DCC_F2);
//  dcc_F5_F8(ADDR, DCC_F7);
  dcc_sleep(5);
  

  cputs("Stat");
//  dsound_system(0);
  dcc_func(ADDR, DCC_FUNC_GRP_2 | DCC_F10);
//  dcc_F9_F12(ADDR, DCC_F10);
  dcc_sleep(10);


  cputs("Mute");
//  dsound_system(0);
  dcc_func(ADDR, DCC_FUNC_GRP_1 | DCC_F8);
  dcc_sleep(5);


//  dsound_system(0);
  cputs("OFF");
  dsound_system(0);

/*
  while (!shutdown_requested())
    {
    switch (state)
      {
      case 1:
        if (SENSOR_3 >= 0x4000)
          {  // button released, toggle headlight;
          funcs ^= DCC_FL;
          state = 0;
          }
        else
          {  // button still held down
          if (time - (int)get_system_up_time() <= 0)
            {  // 1/4 second elapsed, stop loco
            angle = 0;
            ds_rotation_set(&SENSOR_3, 0);
            state = 2;
            }
          }
        break;

      case 2:
        if (SENSOR_3 >= 0x4000)
          // button released, resume
          state = 0;
        break;

      default:
        if (SENSOR_3 >= 0x4000)
          {  // no button pressed, normal throttle operation
          angle = ROTATION_3;

          if (angle < -MAX_ANGLE)
            angle = -MAX_ANGLE;
          else if (angle > MAX_ANGLE)
            angle = MAX_ANGLE;
          }
        else
          {  // button pressed
          time = (int)get_system_up_time() + 250;  // 1/4 second
          state = 1;
          }
        break;
      }

    //dcc_speed14(ADDR, angle * 14 / MAX_ANGLE, funcs & DCC_FL);
    //dcc_speed28(ADDR, angle * 28 / MAX_ANGLE);
    dcc_speed126(ADDR, angle * 126 / MAX_ANGLE);

    dcc_idle();
    dcc_idle();

    dcc_FL_F4(ADDR, funcs);

    dcc_idle();
    dcc_idle();
    }
*/

  cputs("");
  dcc_power_off();

  ds_rotation_off(&SENSOR_3);
  ds_passive(&SENSOR_3);

  dm_mask = old_dm_mask;

  return 0;
  }

// EOF //
