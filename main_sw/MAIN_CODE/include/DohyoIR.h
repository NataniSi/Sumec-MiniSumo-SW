#ifndef DOHYO_H
#define DOHYO_H

#define RAW_BUFFER_LENGTH                               3000
#define TOLERANCE_FOR_DECODERS_MARK_OR_SPACE_MATCHING   30
#include <IRremote.hpp>
#include <UDPStuff.h>


#define DECODE_RC5

class DohyoIR {

    private:

        uint16_t mAddress;
        uint16_t mCommand;
        uint16_t mDohyoID;
        uint8_t mStart;
        uint8_t mGotDohyoID;
        bool enable;

    public:

        DohyoIR(uint_fast8_t pin, bool enable);
        void update();
        uint16_t getDohyoID();
        uint8_t isStarted();
        uint8_t isStopped();
        uint8_t hasDohyoID();
        void removeDohyoID();

};

DohyoIR::DohyoIR(uint_fast8_t pin, bool enable) {

    IrReceiver.begin(pin);
    this->enable = enable;

}

void DohyoIR::update() {

    if (IrReceiver.decode()) {

        if (IrReceiver.decodedIRData.protocol == RC5) {

            IrReceiver.resume();
            IrReceiver.printIRResultShort(&Serial);
            mAddress = IrReceiver.decodedIRData.address;
            mCommand = IrReceiver.decodedIRData.command;

            switch (mAddress) {
                case 0x0B:
                    if (mGotDohyoID == 0) mDohyoID = mCommand & 0xFE;
                    mGotDohyoID = 1;
                    break;
                case 0x07:
                    if ((mCommand & 0xFE) == mDohyoID) {
                        mStart = mCommand & 0x01;
                        mGotDohyoID = 0;
                    }
                    break;
            }
        } else if(IrReceiver.decodedIRData.protocol == SAMSUNG && enable) {

            IrReceiver.resume();
            mCommand = IrReceiver.decodedIRData.command;

            switch (mCommand) {
                case 0x16:
                    mGotDohyoID = 1;
                    break;
                case 0x17:
        
                    mStart = 1;
                    mGotDohyoID = 0;
                    break;
                case 0x1F:
                    
                    mStart = 0;
                    break;
            }
        } else {
            IrReceiver.resume();
            IrReceiver.printIRResultShort(&Serial);
        }

        IrReceiver.resume();
        IrReceiver.printIRResultShort(&Serial);
    }
}

uint8_t DohyoIR::isStarted() {

    return mStart;

}

uint8_t DohyoIR::isStopped() {

    return (mStart == 0);

}

uint16_t DohyoIR::getDohyoID() {

    return mDohyoID;

}

uint8_t DohyoIR::hasDohyoID() {

    return mGotDohyoID;

}

void DohyoIR::removeDohyoID() {

    mGotDohyoID = 0;
    mDohyoID = 0;

}

#endif