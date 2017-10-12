inline float get(float _array[], int r, int c){ return _array[r*2+c]; }
inline void  set(float _array[], int r, int c, float value){ _array[r*2+c] = value;}

void shape_output(const char *path, float landmark[], int number);