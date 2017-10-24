// Classes responsible of writing/reading configuration to/from VMM
// We need
// * A base class that use OPC Client to write/read to FE,
// * Derived classes (VMM etc..) to fill specific parts

#ifndef INC_VMM_H_
#define INC_VMM_H_

class FE {
 private:
  OpcClient m_opc_client;
  /* data */

 public:
  virtual void write(const ptree& params);
  virtual void read();
  FE();
  ~FE();
};

class VMM: public FE {
 private:
  /* data */

 public:
  void write(const ptree& params);
  void read();
  VMM();
  ~VMM();
};
#endif  // INC_VMM_H_

