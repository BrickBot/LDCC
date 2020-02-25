/******************************************************************************
   Author : Mark Riley
     File : ldcc_lnp.c
  Created : 6/29/2003
  Purpose : Demonstration of BrickOS to LDCC comms using LNP
******************************************************************************/

#include <unistd.h>
#include <dsensor.h>
#include <lnp.h>

/******************************************************************************
  LDCC Declarations & Functions
******************************************************************************/

enum
  {
  LDCC_CMD_SPEED = 1,
  LDCC_CMD_FN_OFF,
  LDCC_CMD_FN_ON,
  LDCC_CMD_STOP_ALL,
  LDCC_CMD_POWER_OFF,
  LDCC_CMD_POWER_ON,
  LDCC_CMD_SWITCH_CLOSED,
  LDCC_CMD_SWITCH_THROWN
  };

unsigned char g_packet[8];

#define LDCC_REPEAT 3

void ldcc_send_packet(unsigned char length, char times)
  {
  for ( ; times; times--)
    lnp_addressing_write(g_packet, length, 0x11, 0x01);
  }

void ldcc_power(int state)
  {
  g_packet[0] = state ? LDCC_CMD_POWER_ON : LDCC_CMD_POWER_OFF;
  ldcc_send_packet(1, LDCC_REPEAT);
  }

void ldcc_stop_all()
  {
  g_packet[0] = LDCC_CMD_STOP_ALL;
  ldcc_send_packet(1, LDCC_REPEAT);
  }

void ldcc_speed(char loco, int speed)
  {
  g_packet[0] = LDCC_CMD_SPEED;
  g_packet[1] = loco;
  g_packet[2] = speed >> 8;
  g_packet[3] = speed & 0xFF;
  ldcc_send_packet(4, 1);
  }

void ldcc_function(char loco, char fn, int state)
  {
  g_packet[0] = state ? LDCC_CMD_FN_ON : LDCC_CMD_FN_OFF;
  g_packet[1] = loco;
  g_packet[2] = fn;
  ldcc_send_packet(3, 1);
  }

/******************************************************************************
  Demo
******************************************************************************/

#define MAX_ANGLE 48
#define LOCO 3

int main()
  {
  int angle = 0;
  int last_angle = -1;
  char state = 0;
  char headlight = 0;
  char headlight_repeat = LDCC_REPEAT;
  char speed_repeat = LDCC_REPEAT;
  int time;

  ds_active(&SENSOR_3);
  ds_rotation_on(&SENSOR_3);
  ds_rotation_set(&SENSOR_3, 0);

  ldcc_power(1);

  while (!shutdown_requested())
    {
    switch (state)
      {
      case 1:
        if (SENSOR_3 >= 0x4000)
          {  // button released, toggle headlight;
          headlight = !headlight;
          headlight_repeat = LDCC_REPEAT;
          state = 0;
          }
        else
          {  // button still held down
          if (time - (int)get_system_up_time() <= 0)
            {  // 1/4 second elapsed, stop loco
            ds_rotation_set(&SENSOR_3, 0);
            angle = 0;
            speed_repeat = LDCC_REPEAT;
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

          if (angle != last_angle)
            {
            last_angle = angle;
            speed_repeat = LDCC_REPEAT;
            }
          }
        else
          {  // button pressed
          time = (int)get_system_up_time() + 250;  // 1/4 second
          state = 1;
          }
        break;
      }

    if (speed_repeat)
      {
      speed_repeat--;
      ldcc_speed(LOCO, ((long)angle << 14) / MAX_ANGLE);
      }

    if (headlight_repeat)
      {
      headlight_repeat--;
      ldcc_function(LOCO, 0, headlight);
      }

    // so the man will run...
    msleep(1);
    }

  ldcc_power(0);

  ds_rotation_off(&SENSOR_3);
  ds_passive(&SENSOR_3);

  return 0;
  }

// EOF //
