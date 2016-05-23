#ifndef SIGNAL_H
#define SIGNAL_H

#include <functional>
#include <unordered_map>

// A signal object may call multiple slots with the
// same signature. You can connect functions to the signal
// which will be called when the emit() method on the
// signal object is invoked. Any argument passed to emit()
// will be passed to the given functions.

namespace Enjon { namespace GUI { 

  template <typename... Args>
  class Signal {

    public:

      Signal() : current_id_(0) {}

      // copy creates new signal
      Signal(Signal const& other) : current_id_(0) {}

      // connects a member function of a given object to this Signal
      template <typename F, typename... A>
      int connect_member(F&& f, A&& ... a) const {
        slots_.insert({++current_id_, std::bind(f, a...)});
        return current_id_;
      }

      // connects a std::function to the signal. The returned
      // value can be used to disconnect the function again
      int connect(std::function<void(Args...)> const& slot) const {
        slots_.insert(std::make_pair(++current_id_, slot));
        return current_id_;
      }

      // disconnects a previously connected function
      void disconnect(int id) const {
        slots_.erase(id);
      }

      // disconnects all previously connected functions
      void disconnect_all() const {
        slots_.clear();
      }

      // calls all connected functions
      void emit(Args... p) {
        for(auto it : slots_) {
          it.second(p...);
        }
      }

      // assignment creates new Signal
      Signal& operator=(Signal const& other) {
        disconnect_all();
      }

     private:
      mutable std::unordered_map<int, std::function<void(Args...)>> slots_;
      mutable int current_id_;
  };

}}

#endif /* SIGNAL_H */