#include <node.h>
#include <v8.h>

using namespace v8;

extern Handle<Value> resetTripwireCore();
extern void initCore();

unsigned int tripwireThreshold;
Persistent<Value> context;
int terminated;

void clearTripwire(const FunctionCallbackInfo<Value>& args) 
{
	v8::Isolate* isolate;
  	isolate = args.GetIsolate();
  	
    // Setting tripwireThreshold to 0 indicates to the worker process that
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
	v8::Isolate* isolate;
  	isolate = args.GetIsolate();

	if (0 == args.Length() || !args[0]->IsUint32())
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, 
			"First agument must be an integer time threshold in milliseconds.")));

	if (0 == args[0]->ToUint32()->Value())
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, 
			"The time threshold for blocking operations must be greater than 0.")));

	clearTripwire(args);

	tripwireThreshold = args[0]->ToUint32()->Value();
	if (args.Length() > 1) 
	{
		context = Persistent<Value>::New(isolate, args[1]);
	}

	resetTripwireCore();
}

void getContext(const FunctionCallbackInfo<Value>& args) 
{
    HandleScope scope;

    // If the script had been terminated by tripwire, returns the context passed to resetTripwire;
    // otherwise undefined. This can be used from within the uncaughtException handler to determine
    // whether the exception condition was caused by script termination.

    if (terminated)
    	return context;
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
