#ifndef COORD_TRANSFORM_H
#define COORD_TRANSFORM_H

#include "meschach/matrix.h"

/*
    @brief Transforms a set of coordinates, given in a vector, from a GEI coordinate system to the Cubesat-based coordinate system. Requires that the satellite position vector is non-parallel to the velocity vector. Does not mutate any of the params.
    
    @param given - The coordinates to be transformed.
    @param sat - The coordinates of the satellite.
    @param vel - The velocity vector of the satellite.

    @return A newly allocated VEC pointer (Yes, you are responsible for dealloc) with the cubesat-oriented coordinates 
*/
VEC* transform(VEC* given, VEC* sat, VEC* vel);

#endif /* COORD_TRANSFORM_H */
