#include "dither.h"
#include "../core/macros.h"
#include "../core/serial.h"
#include "math.h"
#include "../module/stepper.h"
#include "../feature/babystep.h"
#include "../../Configuration_adv.h"

uint16_t Dithering::Amplitude = 16; // how many steps for dither
uint16_t Dithering::TimeMS = 100;
float Dithering::MinLayerInterval = 0.1;
float Dithering::PrevZ = 0.0;

uint16_t Dithering::DitherSubTime = 25;
uint8_t Dithering::Ditherssteps[4] = {16, 8, 4, 2};

void Dithering::Handle(float CurrZ)
{

  if (Dithering::Amplitude == 0 || Dithering::TimeMS == 0 || (fabs(CurrZ - Dithering::PrevZ) < Dithering::MinLayerInterval))
  {
    SERIAL_ECHOLNPGM("Delta Z to small, skipping dithering");
    return;
  }
  else
  {

    Dithering::PrevZ = CurrZ;

    while (!Babystep::steps[BS_TODO_AXIS(Z_AXIS)] == 0) // if there are some existing babysteps to do just wait it out
      ;

    uint32_t DitherStartTime = millis();

    int j = 0;

    SERIAL_ECHOLNPGM("Start Dithering...");

    SERIAL_ECHOLNPAIR("DitherStartTime : ", DitherStartTime);

    for (int i = 0; i < 4; i++)
    {

      while (millis() < (DitherStartTime + (Dithering::DitherSubTime * (j + 1))))
      {
        Babystep::add_steps(Z_AXIS, Dithering::Ditherssteps[j]);

        SERIAL_ECHOLNPAIR("DitherStartTime : ", Babystep::steps[BS_TODO_AXIS(Z_AXIS)]);

        while (!Babystep::steps[BS_TODO_AXIS(Z_AXIS)] == 0)
        {
          SERIAL_ECHOLNPAIR("DitherStartTime : ", Babystep::steps[BS_TODO_AXIS(Z_AXIS)]);
        }; //wait

        Babystep::add_steps(Z_AXIS, 2 * -Dithering::Ditherssteps[j]);

        while (!Babystep::steps[BS_TODO_AXIS(Z_AXIS)] == 0) // wait
          ;

        Babystep::add_steps(Z_AXIS, Dithering::Ditherssteps[j]);

        while (!Babystep::steps[BS_TODO_AXIS(Z_AXIS)] == 0) // wait
          ;

        //if (j > 3)
        //{ //for safety
        //  SERIAL_ECHOLNPGM("Dithering Done, J index too high");
        //  return;
        // }
      }
      SERIAL_ECHOLNPAIR("Dither j: ", j);
      SERIAL_ECHOLNPAIR("millis : ", millis());
      j++;
    }
    SERIAL_ECHOLNPGM("Dithering Done.");
  }
}

void Dithering::CalculateParameters()
{
  Dithering::DitherSubTime = Dithering::TimeMS / 4;

  Dithering::Ditherssteps[0] = Dithering::Amplitude;
  Dithering::Ditherssteps[1] = Dithering::Amplitude / 2;
  Dithering::Ditherssteps[2] = Dithering::Amplitude / 4;
  Dithering::Ditherssteps[3] = Dithering::Amplitude / 8;
}
