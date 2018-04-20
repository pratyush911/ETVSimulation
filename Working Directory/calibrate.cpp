cMatrix3d rotate1(cVector3d a,cVector3d b,cVector3d c,cVector3d A,cVector3d B,cVector3d C){ // a,b,c virtual skull ,ABC physical skull 
	cVector3d unit1,unit2,cross,dot, cross_normalise;
	double sin,cos;

	unit1 = normal_find(a,b,c);
	unit2 = normal_find(A,B,C);
	unit1.normalize();
	unit2.normalize();
	unit1.crossr(unit2,cross);
	cross_normalise = cross;
	cross_normalise.normalize()
	cos = unit1.dot(unit2);
	sin = cross.x()/ cross_normalise.x();
	double v1, v2, v3;
	v1 = cross.x();
	v2 = cross.y();
	v3 = cross.z();
	cMatrix3d vx(0, -v3, v2, v3, 0, -v1, -v2, v1, 0);
	cMatrix3d i, vx_2, result;
	i.identity();
	vx.mulr(vx, vx_2);
	i.addr(vx, result);
	result.add(vx_2*(1/1+cos));
	
	return result;



}

double scale1(cVector3d a,cVector3d b,cVector3d A,cVector3d B){
	//double scale;
	//scale = (A.x() - B.x())/(a.x() - b.x());
	double distV = cDistance(a, b);
	double distR = cDistance(A, B);
	return (distR/distV);
}


cVector3d translate1(cVector3d a,cVector3d A){ 
	cVector3d trans;
	A.subr(a,trans);
	return trans	
}

cMatrix3d rotate2(cVector3d A,cVector3d b,cVector3d B,cVector3d normal){
	cVector3d dir1,dir2;
	b.subr(A,dir1);
	B.subr(A,dir2);
	dir1.normalize();
	dir2.normalize();
	double cosvalue,angleradian;
	cosvalue = dir1.dot(dir2);
	angleradian = acos(cosvalue);
	cMatrix3d result;
	result = dir1.cRotAxisAngleRad(normal.x(),normal.y(),normal.z(),angleradian);
	return result;
}

//finding the normals

cVector3d normal_find (cVector3d a,cVector3d b,cVector3d c) {
cVector3d dir0( (a.(x) - b.(x)), (a.(y) - b.(y)), (a.(z) - b.(z)) );
cVector3d dir1((c.(x) - b.(x)), (c.(y) - b.(y)), (c.(z) - b.(z)) );
cVector3d norm;
dir1.crossr(dir0, norm);

return norm;

}

void calibrate3d(cVector3d a,cVector3d b,cVector3d c,cVector3d A,cVector3d B,cVector3d C){
	double scaling_factor;
	cMatrix3d rotate1,rotate2;
	cVector3d translate,normal;
	scaling_factor = scale1(a,b,A,B);
	normal = normal_find(A,B,C);	

	//applying scale to the three points
	a = a*scaling_factor;
	b = b*scaling_factor;
	c = c*scaling_factor;

	// rotate1 = rotate1(a,b,c,A,B,C);
	
	//applying the rotation 1 (aligning the normal vectors of the plane)
	rotate1.mulr(a,a);
	rotate1.mulr(b,b);
	rotate1.mulr(c,c);
	
	translate = translate1(a,A);
	
	//applying the translation (making one point coincide with the physical skull's point)
	a.add(translate);
	b.add(translate);
	c.add(translate);	

	rotate2 = rotate2(a,b,B,normal);
	

	//Applying changes to the mesh
	m_meshVentricles->scaleXYZ(scaling_factor, scaling_factor, scaling_factor);
	m_meshThirdVentricles->scaleXYZ(scaling_factor, scaling_factor, scaling_factor);
	m_meshThalastriateVein->scaleXYZ(scaling_factor, scaling_factor, scaling_factor);
	m_meshChoroidPlexus->scaleXYZ(scaling_factor, scaling_factor, scaling_factor);



}

