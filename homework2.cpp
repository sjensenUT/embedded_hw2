#include <systemc.h>

class OSAPI: public virtual sc_interface
{
public:
  virtual void start() = 0;

  virtual void yield1() = 0;
  virtual void yield2() = 0;


};

class OS1: public virtual sc_channel, public OSAPI
{
  int current;
  sc_event e1, e2;

public:
  OS1(const sc_module_name name): sc_channel(name) { }

  void start() { current = 1; }

  void yield1() { e2.notify(); wait(e1); current = 1; }
  void yield2() { e1.notify(); wait(e2); current = 2; }


};

class OS2: public virtual sc_channel, public OSAPI
{
  int current;
  sc_event e1, e2;

public:
  OS2(const sc_module_name name): sc_channel(name) { }

  void start() { current = 1; }

  void yield1() { e2.notify(SC_ZERO_TIME); wait(e1); current = 1; }
  void yield2() { e1.notify(SC_ZERO_TIME); wait(e2); current = 2; }


};


SC_MODULE(A)
{
  sc_port<OSAPI> os;
  sc_in<bool> e;

  SC_CTOR(A) { SC_THREAD(run); }

  void run(void) {

    cout << "A1: " << sc_time_stamp() << endl;

    wait(10, SC_NS);

    os->yield1();

    cout << "A2: " << sc_time_stamp() << endl;
  
    wait(1, SC_NS);

    while(!e) {
      wait(e->default_event());  
    }

    wait(5, SC_NS);

    os->yield1();

    cout << "A3: " << sc_time_stamp() << endl;

    wait(1, SC_NS);

    os->yield1();

    cout << "A4: " << sc_time_stamp() << endl;

    wait(1, SC_NS);

  }
};

SC_MODULE(B)
{
  sc_port<OSAPI> os;
  sc_out<bool> e;

  SC_CTOR(B) { SC_THREAD(run); }

  void run(void) {

    cout << "B1: " << sc_time_stamp() << endl;

    wait(10, SC_NS);

    os->yield2();

    cout << "B2: " << sc_time_stamp() << endl;
  
    wait(5, SC_NS);

    e = true;

    wait(5, SC_NS);

    os->yield2();

    cout << "B3: " << sc_time_stamp() << endl;

    wait(10, SC_NS);

    os->yield2();

    cout << "B4: " << sc_time_stamp() << endl;

    wait(10, SC_NS);

  }
};

SC_MODULE(Top) {
  sc_signal<bool> e;  
  OS1 os;
  // OS2 os;

  A a;
  B b;

  SC_CTOR(Top): e(false), os("OS"), a("A"), b("B") {
    a.os(os);
    a.e(e);
    b.os(os);
    b.e(e);
  }

  // automatically called by SystemC at the start of simulation (sc_start)
  void start_of_simulation() { os.start(); }
};

int sc_main(int, char*[])
{
  Top top("Top");
  sc_start();
  cout << "Done: " << sc_time_stamp() << endl;
  return 0;
}
