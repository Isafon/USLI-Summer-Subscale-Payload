#include <SD.h>
#include "uSD.h"

bool initSD() {
  return SD.begin(9);
}
