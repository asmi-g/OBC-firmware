#include "coordinate_transform.h"

/*
    @brief Transforms a set of coordinates, given in a vector, from a GEI coordinate system to the Cubesat-based coordinate system. Requires that the satellite position vector is non-parallel to the velocity vector. Does not mutate any of the params.
    
    @param given - The coordinates to be transformed.
    @param sat - The coordinates of the satellite.
    @param vel - The velocity vector of the satellite.

    @return A newly allocated VEC pointer (Yes, you are responsible for dealloc) with the cubesat-oriented coordinates 
*/
VEC* transform(VEC* given, VEC* sat, VEC* vel)
{
    VEC* normal = v_get(3); /* Allocates a 3-element vector to populate the normal with */
    
    /* Then actually find the normal */
    v_set_val(normal, 0, sat->ve[1]*vel->ve[2]-sat->ve[2]*vel->ve[1]);
    v_set_val(normal, 1, sat->ve[2]*vel->ve[0]-sat->ve[0]*vel->ve[0]);
    v_set_val(normal, 2, sat->ve[0]*vel->ve[1]-sat->ve[1]*vel->ve[0]);

    MAT* transformMat = m_get(3,3); /* Allocates a 3x3 matrix for our transformation. */
    
    /* Get all the (Euclidian) norms to save on computation time */
    double velNorm = v_norm2(vel);
    double satNorm = v_norm2(sat);
    double normalNorm = v_norm2(normal);

    /* Create some temp variables to avoid messy code */
    VEC* firstRow = v_copy(vel, VNULL);
    VEC* secondRow = v_copy(normal, VNULL);
    VEC* thirdRow = v_copy(sat, VNULL);
    sv_mlt(1.0/velNorm, firstRow, firstRow);
    sv_mlt(1.0/normalNorm, secondRow, secondRow);
    sv_mlt(-1.0/satNorm, thirdRow, thirdRow);

    /* Fill out each index of the matrix according to the transformation */
    set_row(transformMat, 0, firstRow);
    set_row(transformMat, 1, secondRow);
    set_row(transformMat, 2, thirdRow);
    
    // We must deallocate the normal vector, as well as our temp vectors
    V_FREE(normal);
    V_FREE(firstRow);
    V_FREE(secondRow);
    V_FREE(thirdRow);
    
    VEC* res = v_get(3); /* Allocate a result vector */ 
    v_sub(given, sat, res); /* Translate origin */
    VEC* final = v_get(3); /* mv_mlt does not allow in-situ */
    mv_mlt(transformMat, res, final); /* Multiply by our translation */
    
    M_FREE(transformMat);    
    V_FREE(res);
 
    return final;
}
