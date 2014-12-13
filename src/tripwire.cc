#include <node.h>
#include <v8.h>

using namespace v8;

extern Handle<Value> resetTripwireCore();
extern void initCore();

unsigned int tripwireThreshold;
Persistent<Value> context;
int terminated;

v8::Handle<v8::Value> FortyTwo(const v8::Arguments& args) {
  v8::HandleScope handle_scope;
  return handle_scope.Close(v8::Integer::New(42));
}
In Node v0.12, you write:

1
2
3
4
void FortyTwo(const v8::FunctionCallbackInfo<v8::Value>& info) {
  // Don't need a HandleScope in this particular example.
  info.GetReturnValue().Set(42);
}

void clearTripwire(const FunctionCallbackInfo<Value>& args) 
{

    // Seting tripwireThreshold to 0 indicates to the worker process that
    // there is no threshold to enforce. The worker process will make this determination
    // next time it is signalled, there is no need to force an extra context switch here
    // by explicit signalling. 

	tripwireThreshold = 0;
	terminated = 0;
	context.Dispose();
	context.Clear();

}

void resetTripwire(const FunctionCallbackInfo<Value>& args)
{
    HandleScope scope;

	if (0 == args.Length() || !args[0]->IsUint32())
		return ThrowException(Exception::Error(String::New(
			"First agument must be an integer time threshold in milliseconds.")));

	if (0 == args[0]->ToUint32()->Value())
		return ThrowException(Exception::Error(String::New(
			"The time threshold for blocking operations must be greater than 0.")));

	clearTripwire(args);

	tripwireThreshold = args[0]->ToUint32()->Value();
	if (args.Length() > 1) 
	{
		context = Persistent<Value>::New(args[1]);
	}

	return resetTripwireCore();
}

void getContext(const FunctionCallbackInfo<Value>& args) 
{
    HandleScope scope;

    // If the script had been terminated by tripwire, returns the context passed to resetTripwire;
    // otherwise undefined. This can be used from within the uncaughtException handler to determine
    // whether the exception condition was caused by script termination.

    if (terminated)
    	return context;
    else
    	return Undefined();
}

void init(Handle<Object> target) 
{
	initCore();
  	terminated = 0;
    NODE_SET_METHOD(target, "resetTripwire", resetTripwire);
    NODE_SET_METHOD(target, "clearTripwire", clearTripwire);
    NODE_SET_METHOD(target, "getContext", getContext);
}

NODE_MODULE(tripwire, init);
