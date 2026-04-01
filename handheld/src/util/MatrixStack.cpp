#include "MatrixStack.h"

MatrixStack modelViewStack;
MatrixStack projectionStack;
MatrixStack* currentStack = &modelViewStack;
