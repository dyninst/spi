#ifndef _SPIPC_H_
#define _SPIPC_H_

namespace sp {

class SpIPC {
};

class SpPipe : public SpIPC {
  public:
    SpPipe();
};

class SpTCP : public SpIPC {
};

class SpUDP : public SpIPC {
};

}

#endif /* _SPIPC_H_ */
