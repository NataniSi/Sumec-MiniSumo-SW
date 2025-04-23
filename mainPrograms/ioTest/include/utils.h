#ifndef _UTILS_H
#define _UTILS_H

struct deltaTime {
private:
  uint64_t loopTime[2];
  uint8_t  loopTimeIdx = 0;

public:
  uint64_t loop() {
    loopTimeIdx           = loopTimeIdx ^ 1;
    loopTime[loopTimeIdx] = millis();
    return (loopTime[loopTimeIdx] - loopTime[loopTimeIdx ^ 1]);
  }
  
  uint64_t check() {
    return (loopTime[loopTimeIdx] - loopTime[loopTimeIdx ^ 1]);
  }
};

#endif
