#ifndef MAIN_H
#define MAIN_H

#include "loader/loader.h"
#include <SRDescent/SRDescent.h>

class AsiPlugin : public SRDescent {
	size_t kid;
public:
	explicit AsiPlugin();
	virtual ~AsiPlugin();

protected:
	void onKeyPressed( int key );
};

#endif // MAIN_H
