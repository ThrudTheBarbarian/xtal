
#ifndef __Singleton__
#define __Singleton__

class Singleton
	{
    public:
        static Singleton& getInstance()
        {
            static Singleton    instance; 	// Guaranteed to be destroyed.
											// Instantiated on first use.
            return instance;
        }
        
    private:
        Singleton() {}                    	// Constructor: {} are needed here.

    public:
		NON_COPYABLE(Singleton)

        // Note: Scott Meyers mentions in his Effective Modern
        //       C++ book, that deleted functions should generally
        //       be public as it results in better error messages
        //       due to the compilers behavior to check accessibility
        //       before deleted status
	};


#endif // ! __Singleton__
