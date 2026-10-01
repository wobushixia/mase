#ifndef MASE_H
#define MASE_H

class App {
public:
  virtual ~App() = default;
  void run();

private:
  void Init();
  void Shutdown();

protected:
  virtual void Render() = 0;
};

class Mase : public App {
public:
  static Mase& GetInstance() {
    static Mase mase;
    return mase;
  }

protected:
   void Render() override;
};

#endif // !MASE_H

