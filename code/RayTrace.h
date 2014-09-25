/*
  NOTE: This is the file you will need to begin working with
		You will need to implement the RayTrace::CalculatePixel () function

  This file defines the following:
	RayTrace Class
*/

#ifndef RAYTRACE_H
#define RAYTRACE_H

#include <stdio.h>
#include <stdlib.h>

#include "Utils.h"

#define PI 3.141
#define SAMPLER 3

/*
	RayTrace Class - The class containing the function you will need to implement

	This is the class with the function you need to implement
*/

class RayTrace
{
	float scr_width;
	float scr_height;

	/* 
	 * The rountine transformPoint() transforms the given point with respect to the scale, translate and rotation parameters given
	 * The rotation is not implemented yet.
	 * The order implemented is scale first and then translate. The other cases can also be handled is TBD.
	 */
	Vector transformPoint(Vector point, Vector scale, Vector translate, Vector rotation)
	{
		Vector scaled_point;
		scaled_point.x = point.x*scale.x + translate.x;
		scaled_point.y = point.y*scale.y + translate.y;
		scaled_point.z = point.z*scale.z + translate.z;
		return scaled_point;
	}

	/* 
	 * The rountine ifIntersection() returns the point where the ray intersects an object.
	 * The return vector 'point' will have point.w == 0 if doesn't intersect any object.
	 * This might not be a good way to handle it because transparency is not handled yet.
	 */
	Vector ifIntersection(Vector R0, Vector ray, SceneObject *obj)
	{
		Vector point;
		// Ray equation: P0 + t*ray
		// If the object to check with is a Sphere
		if(obj->IsSphere())
		{
			// Sphere equation: (x-xc)^2 + (y-yc)^2 + (z-zc)^2 = R^2 ::or:: (point - center)^2 = R^2
			// equation to solve t: t^2(ray^2) + 2t(ray(P0-center)) + P0^2 - center^2 - 2*P0*Center - R^2 = 0
			// So we can solve this quadratic equation (a.t^2 + b.t + c = 0) to get the values of t.
			// Vector sc = obj->position; // center of the sphere.
			Vector sc = ((SceneSphere *)obj)->position + ((SceneSphere *)obj)->center; // center of the sphere with location
			float radius = ((SceneSphere *)obj)->radius;
			Vector temp = R0 - sc; // temp = (P0 - center)
			float a = ray.x*ray.x +ray.y*ray.y + ray.z*ray.z;
			float b = 2*(ray.x*temp.x + ray.y*temp.y + ray.z*temp.z);
			// c = P0.x*P0.x + P0.y*P0.y + P0.z*P0.z + sc.x*sc.x + sc.y*sc.y +sc.z*sc.z - 2*(P0.x*sc.x + P0.y*sc.y + P0.z*sc.z) - radius*radius;
			float c = temp.x*temp.x + temp.y*temp.y + temp.z*temp.z - radius*radius;
			float det = b*b - 4*a*c;
			if(det < 0) // no intersection.
				point.w = 0.0f;
			else
			{
				// we compute the two possible values of t.
				float t1 = (-b + sqrt(det))/(2*a);
				float t2 = (-b - sqrt(det))/(2*a);
				if((t1 - EPSILON) > 0 && (t2 - EPSILON) > 0) // EPSILON is to avoid self-intersection.
				{
					// The two intersection point to the sphere are computed.
					Vector T1 = R0 + ray.operator*(t1);
					Vector T2 = R0 + ray.operator*(t2);
					// To check which is the valid we compute the distance of each with respect to the origin.
					Vector temp2 = T1 - R0;
					float dis1 = temp2.x*temp2.x + temp2.y*temp2.y + temp2.z*temp2.z;
					temp2 = T2 - R0;
					float dis2 = temp2.x*temp2.x + temp2.y*temp2.y + temp2.z*temp2.z;
					if(dis1 > dis2)
						point = T2;
					else
						point = T1;
					point.w = 1.0f; // valid intersection. 
				}
				else
					point.w = 0.0f; // no intersection.
			}
		}
		// If the object to check with is a Plane
		else if(obj->IsPlane())
		{
			Vector *p = new Vector[4];
			p[0] = transformPoint(((ScenePlane *)obj)->P[0], ((ScenePlane *)obj)->scale, ((ScenePlane *)obj)->position, ((ScenePlane *)obj)->rotation);
			p[1] = transformPoint(((ScenePlane *)obj)->P[1], ((ScenePlane *)obj)->scale, ((ScenePlane *)obj)->position, ((ScenePlane *)obj)->rotation);
			p[2] = transformPoint(((ScenePlane *)obj)->P[2], ((ScenePlane *)obj)->scale, ((ScenePlane *)obj)->position, ((ScenePlane *)obj)->rotation);
			p[3] = transformPoint(((ScenePlane *)obj)->P[3], ((ScenePlane *)obj)->scale, ((ScenePlane *)obj)->position, ((ScenePlane *)obj)->rotation);
			Vector normal = (p[2] - p[1]).Cross(p[0] - p[1]).Normalize();
			float t = ((p[0] - R0).Dot(normal))/normal.Dot(ray);
			point = R0 + ray.operator*(t); // point of intersection
			Vector temp2 = point - R0;
			float dist = temp2.x*temp2.x + temp2.y*temp2.y + temp2.z*temp2.z;
			if((t - EPSILON)> 0 && point.z < p[0].z && point.z > p[1].z)// &&  point.x < p[0].x && point.x > p[2].x)  // EPSILON is to avoid self-intersection.
				point.w = 1.0f; // valid intersection point
			else
				point.w = 0.0f; // no intersection point
			delete[] p;
		}
		// If the object to check with is a Triangle
		else if(obj->IsTriangle())
		{
			Vector *v = new Vector[3];
			v[0] = transformPoint(((SceneTriangle *)obj)->vertex[0], ((SceneTriangle *)obj)->scale, ((SceneTriangle *)obj)->position, ((SceneTriangle *)obj)->rotation);
			Vector n0 = ((SceneTriangle *)obj)->normal[0];
			v[1] = transformPoint(((SceneTriangle *)obj)->vertex[1], ((SceneTriangle *)obj)->scale, ((SceneTriangle *)obj)->position, ((SceneTriangle *)obj)->rotation);
			Vector n1 = ((SceneTriangle *)obj)->normal[1];
			v[2] = transformPoint(((SceneTriangle *)obj)->vertex[2], ((SceneTriangle *)obj)->scale, ((SceneTriangle *)obj)->position, ((SceneTriangle *)obj)->rotation);
			Vector n2 = ((SceneTriangle *)obj)->normal[2];
			Vector normal = (v[1] - v[0]).Cross(v[2] - v[0]).Normalize();
			float t = ((v[0] - R0).Dot(normal))/normal.Dot(ray);
			if((t - EPSILON) > 0) // EPSILON is to avoid self-intersection.
			{
				point = R0 + ray.operator*(t);

				Vector area1 = (v[0] - point).Cross(v[1] - point);
				Vector area2 = (v[1] - point).Cross(v[2] - point);
				Vector area3 = (v[2] - point).Cross(v[0] - point);
				Vector tarea = (v[0] - v[2]).Cross(v[1] - v[2]);

				if((area1.Dot(tarea) - EPSILON) < 0 || (area2.Dot(tarea) - EPSILON) < 0 || (area3.Dot(tarea) - EPSILON) < 0) // EPSILON is to avoid self-intersection.
					point.w = 0.0f; // valid intersection point
				else
					point.w = 1.0f; // no intersection point
			}
			else
				point.w = 0.0f; // no intersection point
			delete[] v;
		}
		// If the object to check with is a Model
		else if(obj->IsModel())
		{
			unsigned int numTris = ((SceneModel *)obj)->GetNumTriangles ();
			Vector *v = new Vector[3];
			Vector tempPoint;
			bool point_flag = false;
			// checking for all the Triangles in the model
			for (unsigned int n = 0; n < numTris; n++)
			{
				SceneTriangle *sceneTri = ((SceneModel *)obj)->GetTriangle(n);
				v[0] = transformPoint(sceneTri->vertex[0], ((SceneModel *)obj)->scale, ((SceneModel *)obj)->position, ((SceneModel *)obj)->rotation);
				v[1] = transformPoint(sceneTri->vertex[1], ((SceneModel *)obj)->scale, ((SceneModel *)obj)->position, ((SceneModel *)obj)->rotation);
				v[2] = transformPoint(sceneTri->vertex[2], ((SceneModel *)obj)->scale, ((SceneModel *)obj)->position, ((SceneModel *)obj)->rotation);
				Vector normal = (v[1] - v[0]).Cross(v[2] - v[0]).Normalize();
				float t = ((v[0] - R0).Dot(normal))/normal.Dot(ray);
				if((t - EPSILON) > 0 && t < 10000) // EPSILON is to avoid self-intersection.
				{
					tempPoint = R0 + ray.operator*(t);

					Vector area1 = (v[0] - tempPoint).Cross(v[1] - tempPoint);
					Vector area2 = (v[1] - tempPoint).Cross(v[2] - tempPoint);
					Vector area3 = (v[2] - tempPoint).Cross(v[0] - tempPoint);
					Vector tarea = (v[0] - v[2]).Cross(v[1] - v[2]);

					if(!((area1.Dot(tarea) - EPSILON) < 0 || (area2.Dot(tarea) - EPSILON) < 0 || (area3.Dot(tarea) - EPSILON) < 0)) // EPSILON is to avoid self-intersection.
					{
						if(point_flag == true)
						{
							float dis1 = tempPoint.x*tempPoint.x + tempPoint.y*tempPoint.y + tempPoint.z*tempPoint.z;
							float dis2 = point.x*point.x + point.y*point.y + point.z*point.z;
							if(dis1 < dis2) // checking for the nearest point
							{
								tempPoint.w = 1.0f; // valid intersection point
								point = tempPoint;
							}
							break;
						}
						else
						{
							tempPoint.w = 1.0f; // valid intersection point
							point = tempPoint;
							point_flag = true;
						}
					}
					else
						tempPoint.w = 0.0f; // no intersection point
				}
				else
					tempPoint.w = 0.0f; // no intersection point
			}
			if(!point_flag)
				point.w = 0.0f; // no intersection point
			delete[] v;
		}
		else
		{
			printf ("Ohh C'mon!! You gotta be kidding me...\n");
			point.w = 0.0f;
		}
		return point;
	}

	/* 
	 * The rountine ifLight() returns whether the point considered is getting light or is in shadow.
	 * Return 'false' is no intersection and 'true' if it is in shadow.
	 */
	bool ifLight(Vector R0, Vector ray, SceneObject *obj)
	{
		bool return_flag = false;
		// Ray equation: P0 + t*ray
		if(obj->IsSphere())
		{
			// Sphere equation: (x-xc)^2 + (y-yc)^2 + (z-zc)^2 = R^2 ::or:: (point - center)^2 = R^2
			// equation to solve t: t^2(ray^2) + 2t(ray(P0-center)) + P0^2 - center^2 - 2*P0*Center - R^2 = 0
			// So we can solve this quadratic equation (a.t^2 + b.t + c = 0) to get the values of t.
			// Vector sc = obj->position; // center of the sphere.
			Vector sc = ((SceneSphere *)obj)->position + ((SceneSphere *)obj)->center; // center of the sphere with location
			float radius = ((SceneSphere *)obj)->radius;
			Vector temp = R0 - sc; // temp = (P0 - center)
			float a = ray.x*ray.x +ray.y*ray.y + ray.z*ray.z;
			float b = 2*(ray.x*temp.x + ray.y*temp.y + ray.z*temp.z);
			// c = P0.x*P0.x + P0.y*P0.y + P0.z*P0.z + sc.x*sc.x + sc.y*sc.y +sc.z*sc.z - 2*(P0.x*sc.x + P0.y*sc.y + P0.z*sc.z) - radius*radius;
			float c = temp.x*temp.x + temp.y*temp.y + temp.z*temp.z - radius*radius;
			float det = b*b - 4*a*c;
			if(det < 0) // no intersection.
				return_flag = false;
			else
			{
				// we compute the two possible values of t.
				float t1 = (-b + sqrt(det))/(2*a);
				float t2 = (-b - sqrt(det))/(2*a);
				if(t1 <=0 || t2<=0) // t cannot be negative.
					return_flag = false; // no intersection with another object.
				else
					return_flag = true; // intersection with another object.
			}
		}
		else if(obj->IsPlane())
		{
			Vector *p = new Vector[4];
			p[0] = transformPoint(((ScenePlane *)obj)->P[0], ((ScenePlane *)obj)->scale, ((ScenePlane *)obj)->position, ((ScenePlane *)obj)->rotation);
			p[1] = transformPoint(((ScenePlane *)obj)->P[1], ((ScenePlane *)obj)->scale, ((ScenePlane *)obj)->position, ((ScenePlane *)obj)->rotation);
			p[2] = transformPoint(((ScenePlane *)obj)->P[2], ((ScenePlane *)obj)->scale, ((ScenePlane *)obj)->position, ((ScenePlane *)obj)->rotation);
			p[3] = transformPoint(((ScenePlane *)obj)->P[3], ((ScenePlane *)obj)->scale, ((ScenePlane *)obj)->position, ((ScenePlane *)obj)->rotation);
			Vector normal = (p[2] - p[1]).Cross(p[0] - p[1]).Normalize();
			float t = ((p[0] - R0).Dot(normal))/normal.Dot(ray);
			Vector point = R0 + ray.operator*(t);
			if((t - EPSILON) <= 0) // EPSILON is to avoid self-intersection.
				return_flag = false; // no intersection with another object.
			else
				return_flag = true; // intersection with another object.
			delete[] p;
		}
		else if(obj->IsTriangle())
		{
			Vector *v = new Vector[3];
			v[0] = transformPoint(((SceneTriangle *)obj)->vertex[0], ((SceneTriangle *)obj)->scale, ((SceneTriangle *)obj)->position, ((SceneTriangle *)obj)->rotation);
			Vector n0 = ((SceneTriangle *)obj)->normal[0];
			v[1] = transformPoint(((SceneTriangle *)obj)->vertex[1], ((SceneTriangle *)obj)->scale, ((SceneTriangle *)obj)->position, ((SceneTriangle *)obj)->rotation);
			Vector n1 = ((SceneTriangle *)obj)->normal[1];
			v[2] = transformPoint(((SceneTriangle *)obj)->vertex[2], ((SceneTriangle *)obj)->scale, ((SceneTriangle *)obj)->position, ((SceneTriangle *)obj)->rotation);
			Vector n2 = ((SceneTriangle *)obj)->normal[2];
			Vector normal = (v[1] - v[0]).Cross(v[2] - v[0]).Normalize();
			float t = ((v[0] - R0).Dot(normal))/normal.Dot(ray);
			if((t - EPSILON) > 0) // EPSILON is to avoid self-intersection.
			{
				Vector point = R0 + ray.operator*(t);

				Vector area1 = (v[0] - point).Cross(v[1] - point);
				Vector area2 = (v[1] - point).Cross(v[2] - point);
				Vector area3 = (v[2] - point).Cross(v[0] - point);
				Vector tarea = (v[0] - v[2]).Cross(v[1] - v[2]);
			
				if(area1.Dot(tarea) < 0 || area2.Dot(tarea) < 0 || area3.Dot(tarea) < 0)
					return_flag = false; // no intersection with another object.
				else
					return_flag = true; // intersection with another object.
			}
			return_flag = false; // no intersection with another object.
			delete[] v;
		}
		else if(obj->IsModel())
		{
			unsigned int numTris = ((SceneModel *)obj)->GetNumTriangles ();
			Vector *v = new Vector[3];
			for (unsigned int n = 0; n < numTris; n++)
			{
				SceneTriangle *sceneTri = ((SceneModel *)obj)->GetTriangle(n);
				v[0] = transformPoint(sceneTri->vertex[0], ((SceneModel *)obj)->scale, ((SceneModel *)obj)->position, ((SceneModel *)obj)->rotation);
				v[1] = transformPoint(sceneTri->vertex[1], ((SceneModel *)obj)->scale, ((SceneModel *)obj)->position, ((SceneModel *)obj)->rotation);
				v[2] = transformPoint(sceneTri->vertex[2], ((SceneModel *)obj)->scale, ((SceneModel *)obj)->position, ((SceneModel *)obj)->rotation);
				Vector normal = (v[1] - v[0]).Cross(v[2] - v[0]).Normalize();
				float t = ((v[0] - R0).Dot(normal))/normal.Dot(ray);
				if((t - EPSILON) > 0) // EPSILON is to avoid self-intersection.
				{
					Vector tempPoint = R0 + ray.operator*(t);

					Vector area1 = (v[0] - tempPoint).Cross(v[1] - tempPoint);
					Vector area2 = (v[1] - tempPoint).Cross(v[2] - tempPoint);
					Vector area3 = (v[2] - tempPoint).Cross(v[0] - tempPoint);
					Vector tarea = (v[0] - v[2]).Cross(v[1] - v[2]);

					if(area1.Dot(tarea) < 0 || area2.Dot(tarea) < 0 || area3.Dot(tarea) < 0)
						return_flag = false;
					else
					{
						return_flag = true;
						break;
					}
				}
				else
					return_flag = false;
			}
			delete[] v;
		}
		return return_flag;
	}

	/* 
	 * The rountine traceLight() calls rountine ifLight() to check if point is in shadow or light.
	 * Return 'false' for shadow and 'true' for light.
	 */
	bool traceLight(Vector P0, Vector ray, SceneObject* Obj)
	{
		bool colour = true;

		// Get the Objects
		unsigned int numObjs = m_Scene.GetNumObjects();
		for (unsigned int n = 0; n < numObjs; n++)
		{
			SceneObject *sceneObj = m_Scene.GetObjectA(n);
			if(ifLight(P0, ray, sceneObj))
			{
				colour = false;
				break;
			}
		}
		return colour;
	}

	/* 
	 * The rountine trace() computes and returns the color of the point corresponding to the ray
	 */
	Vector trace(Vector P0, Vector ray, float ref_index, int depth)
	{
		Vector color = Vector(0.0f, 0.0f, 0.0f);
		if(depth > 0)
		{
			SceneBackground sceneBg = m_Scene.GetBackground();
			bool point_flag = false;
			Vector point;
			SceneObject *sceneObj;

			// Get the Objects
			unsigned int numObjs = m_Scene.GetNumObjects();
			for (unsigned int n = 0; n < numObjs; n++)
			{
				SceneObject *tempSceneObj = m_Scene.GetObjectA(n);
				Vector tempPoint = ifIntersection(P0, ray, tempSceneObj);
				if(tempPoint.w)
				{
					if(point_flag) // we check for the closest intersecting object.
					{
						Vector D1 = point - P0;
						Vector D2 = tempPoint - P0;
						float d1 = D1.x*D1.x + D1.y*D1.y + D1.z*D1.z;
						float d2 = D2.x*D2.x + D2.y*D2.y + D2.z*D2.z;
						if(d1 > d2)
						{
							point = tempPoint;
							sceneObj = tempSceneObj;
						}
					}
					else
					{
						sceneObj = tempSceneObj;
						point = tempPoint;
						point_flag = true;
					}
				}
			}
			// point_flag is true only if there is an intersection with an object
			if(point_flag && sceneObj->IsSphere())
			{
				Vector sc = ((SceneSphere *)sceneObj)->position + ((SceneSphere *)sceneObj)->center; // center of the sphere with location
				Vector normal = (point - sc).Normalize();
				SceneMaterial sceneMat = m_Scene.GetMaterial(((SceneSphere *)sceneObj)->material);
				unsigned int numLights = m_Scene.GetNumLights();
				for (unsigned int n = 0; n < numLights; n++)
				{
					SceneLight lightObj = m_Scene.GetLight(n);
					Vector recLight = (lightObj.position - point).Normalize();
					Vector lightNormal = recLight.Cross(normal).Normalize();
					for(float i = 0.0; i < 4.0*SAMPLER; i++) // logic for soft shadow.
					{
						Vector scrap = recLight.Cross(lightNormal).Normalize();
						lightNormal = (scrap + lightNormal.operator*(SAMPLER-1)).Normalize();
						Vector lightSampler = (recLight.operator*(30) + lightNormal).Normalize();
						bool color_flag = traceLight(point, recLight, sceneObj);
						if(color_flag)
						{
							float temp_color = (recLight.Dot(normal));
							if(temp_color < 0)
								temp_color = 0;
							Vector temp_reflection = (normal.operator*(2*(recLight.Dot(normal))) - recLight).Normalize();
							float specular_color = temp_reflection.Dot(ray.operator*(-1));
							if(specular_color < 0)
								specular_color = 0;
							else
								specular_color = pow(specular_color, sceneMat.shininess);
							color.x = color.x + lightObj.diffuse.x*sceneMat.diffuse.x*temp_color + lightObj.specular.x*sceneMat.specular.x*specular_color;
							color.y = color.y + lightObj.diffuse.y*sceneMat.diffuse.y*temp_color + lightObj.specular.y*sceneMat.specular.y*specular_color;
							color.z = color.z + lightObj.diffuse.z*sceneMat.diffuse.z*temp_color + lightObj.specular.z*sceneMat.specular.z*specular_color;
							//color = color.operator/(2);
						}
						else
							color = color + Vector(0.0f, 0.0f, 0.0f);
					}
				}
				color = color.operator/(numLights*4.0*SAMPLER);
				// Reflection.
				if((depth-1) > 0 && (sceneMat.reflective.x + sceneMat.reflective.y + sceneMat.reflective.z > 0))
				{
					Vector reflection = (ray - normal.operator*(2*(ray.Dot(normal)))).Normalize();
					Vector refColor = trace(point, reflection, ref_index, (depth-1));
					color.x = color.x + sceneMat.reflective.x*refColor.x;
					color.y = color.y + sceneMat.reflective.y*refColor.y;
					color.z = color.z + sceneMat.reflective.z*refColor.z;
				}
				// Refraction.
				if((depth-1) > 0 && (sceneMat.refraction.x + sceneMat.refraction.y + sceneMat.refraction.z > 0))
				{
					float n12 = ref_index/sceneMat.refraction_index;
					float sqroot = 1-n12*n12*(1 - ray.Dot(normal)*ray.Dot(normal));
					if(sqroot < 0)
						sqroot = 0;
					Vector refract_vector = (ray.operator*(n12) + normal.operator*(n12*(ray.Dot(normal) - sqrt(sqroot)))).Normalize();
					Vector refColor = trace(point, refract_vector, sceneMat.refraction_index, (depth-1));
					color.x = color.x + sceneMat.refraction.x*refColor.x;
					color.y = color.y + sceneMat.refraction.y*refColor.y;
					color.z = color.z + sceneMat.refraction.z*refColor.z;
				}
			}
			else if(point_flag && sceneObj->IsPlane())
			{
				Vector *p = new Vector[4];
				p[0] = transformPoint(((ScenePlane *)sceneObj)->P[0], ((ScenePlane *)sceneObj)->scale, ((ScenePlane *)sceneObj)->position, ((ScenePlane *)sceneObj)->rotation);
				p[1] = transformPoint(((ScenePlane *)sceneObj)->P[1], ((ScenePlane *)sceneObj)->scale, ((ScenePlane *)sceneObj)->position, ((ScenePlane *)sceneObj)->rotation);
				p[2] = transformPoint(((ScenePlane *)sceneObj)->P[2], ((ScenePlane *)sceneObj)->scale, ((ScenePlane *)sceneObj)->position, ((ScenePlane *)sceneObj)->rotation);
				p[3] = transformPoint(((ScenePlane *)sceneObj)->P[3], ((ScenePlane *)sceneObj)->scale, ((ScenePlane *)sceneObj)->position, ((ScenePlane *)sceneObj)->rotation);
				Vector normal = (p[2] - p[1]).Cross(p[0] - p[1]).Normalize();
				SceneMaterial sceneMat = m_Scene.GetMaterial(((ScenePlane *)sceneObj)->material);
				unsigned int numLights = m_Scene.GetNumLights();
				for (unsigned int n = 0; n < numLights; n++)
				{
					SceneLight lightObj = m_Scene.GetLight(n);
					Vector recLight = (lightObj.position - point).Normalize();
					Vector lightNormal = recLight.Cross(normal).Normalize();
					for(float i = 0.0; i < 4.0*SAMPLER; i++) // logic for soft shadow.
					{
						Vector scrap = recLight.Cross(lightNormal).Normalize();
						lightNormal = (scrap + lightNormal.operator*(SAMPLER-1)).Normalize();
						for(int j = 1; j < 4; j++)
						{
							Vector lightSampler = (recLight.operator*(10+7.5*j) + lightNormal).Normalize();
							bool color_flag = traceLight(point, lightSampler, sceneObj); // if false, it is in shadow
							if(!color_flag)
								color = color + Vector(0.0f, 0.0f, 0.0f);
							else if(((ScenePlane *)sceneObj)->texture == "checkerboard")
							{
								int s = (int)point.x;
								int t = (int)point.z;
								bool flag = false;
								if((point.x >= 0 && point.z >=0) || (point.x <= 0 && point.z <=0))
									if(((s%2 == s%4) && (t%2 == t%4)) || (!(s%2 == s%4) && !(t%2 == t%4)))
										color = color + Vector(0.9f, 0.9f, 0.9f);
									else
										color = color + sceneMat.diffuse;
								else
									if(((s%2 == s%4) && (t%2 == t%4)) || (!(s%2 == s%4) && !(t%2 == t%4)))
										color = color + sceneMat.diffuse;
									else
										color = color + Vector(0.9f, 0.9f, 0.9f);
							}
							else
							{
								float temp_color = (recLight.Dot(normal));
								if(temp_color < 0)
									temp_color = 0;
								color.x = color.x + lightObj.color.x*sceneMat.diffuse.x*temp_color;
								color.y = color.y + lightObj.color.y*sceneMat.diffuse.y*temp_color;
								color.z = color.z + lightObj.color.z*sceneMat.diffuse.z*temp_color;
							}
						}
					}
				}
				color = color.operator/(numLights*12.0*SAMPLER);
				// Reflection
				if((depth-1) > 0 && (sceneMat.reflective.x + sceneMat.reflective.y + sceneMat.reflective.z > 0))
				{
					Vector reflection = (ray - normal.operator*(2*(ray.Dot(normal)))).Normalize();
					Vector refColor = trace(point, reflection, ref_index, (depth-1));
					color.x = color.x + sceneMat.reflective.x*refColor.x;
					color.y = color.y + sceneMat.reflective.y*refColor.y;
					color.z = color.z + sceneMat.reflective.z*refColor.z;
					//color = color.operator/(2);
				}
				// Refraction.
				if((depth-1) > 0 && (sceneMat.refraction.x + sceneMat.refraction.y + sceneMat.refraction.z > 0))
				{
					float n12 = ref_index/sceneMat.refraction_index;
					float sqroot = 1-n12*n12*(1 - ray.Dot(normal)*ray.Dot(normal));
					if(sqroot < 0)
						sqroot = 0;
					Vector refract_vector = (ray.operator*(n12) + normal.operator*(n12*(ray.Dot(normal) - sqrt(sqroot)))).Normalize();
					Vector refColor = trace(point, refract_vector, sceneMat.refraction_index, (depth-1));
					color.x = color.x + sceneMat.refraction.x*refColor.x;
					color.y = color.y + sceneMat.refraction.y*refColor.y;
					color.z = color.z + sceneMat.refraction.z*refColor.z;
				}
				delete[] p;
			}
			else if(point_flag && sceneObj->IsTriangle())
			{
				Vector *v = new Vector[3];
				v[0] = transformPoint(((SceneTriangle *)sceneObj)->vertex[0], ((SceneTriangle *)sceneObj)->scale, ((SceneTriangle *)sceneObj)->position, ((SceneTriangle *)sceneObj)->rotation);
				Vector n0 = ((SceneTriangle *)sceneObj)->normal[0];
				SceneMaterial sceneMat0 = m_Scene.GetMaterial(((SceneTriangle *)sceneObj)->material[0]);

				v[1] = transformPoint(((SceneTriangle *)sceneObj)->vertex[1], ((SceneTriangle *)sceneObj)->scale, ((SceneTriangle *)sceneObj)->position, ((SceneTriangle *)sceneObj)->rotation);
				Vector n1 = ((SceneTriangle *)sceneObj)->normal[1];
				SceneMaterial sceneMat1 = m_Scene.GetMaterial(((SceneTriangle *)sceneObj)->material[1]);

				v[2] = transformPoint(((SceneTriangle *)sceneObj)->vertex[2], ((SceneTriangle *)sceneObj)->scale, ((SceneTriangle *)sceneObj)->position, ((SceneTriangle *)sceneObj)->rotation);
				Vector n2 = ((SceneTriangle *)sceneObj)->normal[2];
				SceneMaterial sceneMat2 = m_Scene.GetMaterial(((SceneTriangle *)sceneObj)->material[2]);
				
				float area1 = (v[0] - point).Cross(v[1] - point).Magnitude()/2;
				float area2 = (v[1] - point).Cross(v[2] - point).Magnitude()/2;
				float area3 = (v[2] - point).Cross(v[0] - point).Magnitude()/2;
				// we compute the weighted normal.
				Vector normal = (n2.operator*(area1) + n0.operator*(area2) + n1.operator*(area3))/(area1 + area2 + area3);
				SceneMaterial *sceneMat = new SceneMaterial;
				// we compute the weighted material properties.
				sceneMat->diffuse = (sceneMat2.diffuse.operator*(area1) + sceneMat0.diffuse.operator*(area2) + sceneMat1.diffuse.operator*(area3))/(area1 + area2 + area3);
				sceneMat->reflective = (sceneMat2.reflective.operator*(area1) + sceneMat0.reflective.operator*(area2) + sceneMat1.reflective.operator*(area3))/(area1 + area2 + area3);
				sceneMat->specular = (sceneMat2.specular.operator*(area1) + sceneMat0.specular.operator*(area2) + sceneMat1.specular.operator*(area3))/(area1 + area2 + area3);
				sceneMat->refraction = (sceneMat2.refraction.operator*(area1) + sceneMat0.refraction.operator*(area2) + sceneMat1.refraction.operator*(area3))/(area1 + area2 + area3);
				sceneMat->shininess = sceneMat2.shininess;
				sceneMat->refraction_index = sceneMat2.refraction_index;

				unsigned int numLights = m_Scene.GetNumLights();
				for (unsigned int n = 0; n < numLights; n++)
				{
					SceneLight lightObj = m_Scene.GetLight(n);
					Vector recLight = (lightObj.position - point).Normalize();
					Vector lightNormal = recLight.Cross(normal).Normalize();
					for(float i = 0.0; i < 4.0*SAMPLER; i++) // logic for soft shadow.
					{
						Vector scrap = recLight.Cross(lightNormal).Normalize();
						lightNormal = (scrap + lightNormal.operator*(SAMPLER-1)).Normalize();
						Vector lightSampler = (recLight.operator*(30) + lightNormal).Normalize();
						bool color_flag = traceLight(point, lightSampler, sceneObj); // if false, it is in shadow
						if(!color_flag)
							color = color + Vector(0.0f, 0.0f, 0.0f);
						else
						{
							float temp_color = (recLight.Dot(normal));
							if(temp_color < 0)
								temp_color = 0;
							Vector temp_reflection = (normal.operator*(2*(recLight.Dot(normal))) - recLight).Normalize();
							float specular_color = temp_reflection.Dot((point - P0).Normalize());
							if(specular_color < 0)
								specular_color = 0;
							else
								specular_color = pow(specular_color, sceneMat->shininess);
							color.x = color.x + lightObj.diffuse.x*sceneMat->diffuse.x*temp_color + lightObj.specular.x*sceneMat->specular.x*specular_color;
							color.y = color.y + lightObj.diffuse.y*sceneMat->diffuse.y*temp_color + lightObj.specular.y*sceneMat->specular.y*specular_color;
							color.z = color.z + lightObj.diffuse.z*sceneMat->diffuse.z*temp_color + lightObj.specular.z*sceneMat->specular.z*specular_color;
							//color = sceneMat->diffuse;
						}
					}
				}
				color = color.operator/(numLights*4.0*SAMPLER);
				// Reflection
				if((depth-1) > 0 && (sceneMat->reflective.x + sceneMat->reflective.y + sceneMat->reflective.z > 0))
				{
					Vector reflection = (ray - normal.operator*(2*(ray.Dot(normal)))).Normalize();
					Vector refColor = trace(point, reflection, ref_index, (depth-1));
					color.x = color.x + sceneMat->reflective.x*refColor.x;
					color.y = color.y + sceneMat->reflective.y*refColor.y;
					color.z = color.z + sceneMat->reflective.z*refColor.z;
					color = color.operator/(2);
				}
				// Refraction.
				if((depth-1) > 0 && (sceneMat->refraction.x + sceneMat->refraction.y + sceneMat->refraction.z > 0))
				{
					float n12 = ref_index/sceneMat->refraction_index;
					float sqroot = 1-n12*n12*(1 - ray.Dot(normal)*ray.Dot(normal));
					if(sqroot < 0)
						sqroot = 0;
					Vector refract_vector = (ray.operator*(n12) + normal.operator*(n12*(ray.Dot(normal) - sqrt(sqroot)))).Normalize();
					Vector refColor = trace(point, refract_vector, sceneMat->refraction_index, (depth-1));
					color.x = color.x + sceneMat->refraction.x*refColor.x;
					color.y = color.y + sceneMat->refraction.y*refColor.y;
					color.z = color.z + sceneMat->refraction.z*refColor.z;
				}
				//delete[] sceneMat;
				delete[] v;
			}
			else if(point_flag && sceneObj->IsModel())
			{
				unsigned int numTris = ((SceneModel *)sceneObj)->GetNumTriangles ();
				Vector *v = new Vector[3];
				SceneTriangle *sceneTri;
				for (unsigned int n = 0; n < numTris; n++) // compute index of the triangle which is intersecting.
				{
					sceneTri = ((SceneModel *)sceneObj)->GetTriangle(n);
					v[0] = transformPoint(sceneTri->vertex[0], ((SceneModel *)sceneObj)->scale, ((SceneModel *)sceneObj)->position, ((SceneModel *)sceneObj)->rotation);
					v[1] = transformPoint(sceneTri->vertex[1], ((SceneModel *)sceneObj)->scale, ((SceneModel *)sceneObj)->position, ((SceneModel *)sceneObj)->rotation);
					v[2] = transformPoint(sceneTri->vertex[2], ((SceneModel *)sceneObj)->scale, ((SceneModel *)sceneObj)->position, ((SceneModel *)sceneObj)->rotation);
					Vector tempNormal = (v[1] - v[0]).Cross(v[2] - v[0]).Normalize();
					float t = ((v[0] - P0).Dot(tempNormal))/tempNormal.Dot(ray);
					if((t - 0.0001) > 0 && t < 10000)
					{
						Vector tempPoint = P0 + ray.operator*(t);

						Vector area1 = (v[0] - tempPoint).Cross(v[1] - tempPoint);
						Vector area2 = (v[1] - tempPoint).Cross(v[2] - tempPoint);
						Vector area3 = (v[2] - tempPoint).Cross(v[0] - tempPoint);
						Vector tarea = (v[0] - v[2]).Cross(v[1] - v[2]);

						if(!((area1.Dot(tarea) - 0.0001) < 0 || (area2.Dot(tarea) - 0.0001) < 0 || (area3.Dot(tarea) - 0.0001) < 0))
							if(tempPoint.x == point.x && tempPoint.y == point.y && tempPoint.z == point.z)
								break;
					}
				}
				Vector n0 = sceneTri->normal[0];
				SceneMaterial sceneMat0 = m_Scene.GetMaterial(sceneTri->material[0]);

				Vector n1 = sceneTri->normal[1];
				SceneMaterial sceneMat1 = m_Scene.GetMaterial(sceneTri->material[1]);

				Vector n2 = sceneTri->normal[2];
				SceneMaterial sceneMat2 = m_Scene.GetMaterial(sceneTri->material[2]);
				
				float area1 = (v[0] - point).Cross(v[1] - point).Magnitude()/2;
				float area2 = (v[1] - point).Cross(v[2] - point).Magnitude()/2;
				float area3 = (v[2] - point).Cross(v[0] - point).Magnitude()/2;
				// we compute the weighted normal
				Vector normal = (n2.operator*(area1) + n0.operator*(area2) + n1.operator*(area3))/(area1 + area2 + area3);
				SceneMaterial *sceneMat = new SceneMaterial;
				// we compute the weighted material properties.
				sceneMat->diffuse = (sceneMat2.diffuse.operator*(area1) + sceneMat0.diffuse.operator*(area2) + sceneMat1.diffuse.operator*(area3))/(area1 + area2 + area3);
				sceneMat->reflective = (sceneMat2.reflective.operator*(area1) + sceneMat0.reflective.operator*(area2) + sceneMat1.reflective.operator*(area3))/(area1 + area2 + area3);
				sceneMat->specular = (sceneMat2.specular.operator*(area1) + sceneMat0.specular.operator*(area2) + sceneMat1.specular.operator*(area3))/(area1 + area2 + area3);
				sceneMat->refraction = (sceneMat2.refraction.operator*(area1) + sceneMat0.refraction.operator*(area2) + sceneMat1.refraction.operator*(area3))/(area1 + area2 + area3);
				sceneMat->shininess = sceneMat2.shininess;
				sceneMat->refraction_index = sceneMat2.refraction_index;

				unsigned int numLights = m_Scene.GetNumLights();
				for (unsigned int n = 0; n < numLights; n++)
				{
					SceneLight lightObj = m_Scene.GetLight(n);
					Vector recLight = (lightObj.position - point).Normalize();
					Vector lightNormal = recLight.Cross(normal).Normalize();
					for(float i = 0.0; i < 4.0*SAMPLER; i++) // logic for soft shadow.
					{
						Vector scrap = recLight.Cross(lightNormal).Normalize();
						lightNormal = (scrap + lightNormal.operator*(SAMPLER-1)).Normalize();
						Vector lightSampler = (recLight.operator*(30) + lightNormal).Normalize();
						bool color_flag = traceLight(point, lightSampler, sceneObj); // if false, it is in shadow
						if(!color_flag)
							color = color + Vector(0.0f, 0.0f, 0.0f);
						else
						{
							float temp_color = (recLight.Dot(normal));
							if(temp_color < 0)
								temp_color = 0;
							Vector temp_reflection = (normal.operator*(2*(recLight.Dot(normal))) - recLight).Normalize();
							float specular_color = temp_reflection.Dot((point - P0).Normalize());
							if(specular_color < 0)
								specular_color = 0;
							else
								specular_color = pow(specular_color, sceneMat->shininess);
							color.x = color.x + lightObj.diffuse.x*sceneMat->diffuse.x*temp_color + lightObj.specular.x*sceneMat->specular.x*specular_color;
							color.y = color.y + lightObj.diffuse.y*sceneMat->diffuse.y*temp_color + lightObj.specular.y*sceneMat->specular.y*specular_color;
							color.z = color.z + lightObj.diffuse.z*sceneMat->diffuse.z*temp_color + lightObj.specular.z*sceneMat->specular.z*specular_color;
							//color = color.operator/(2);
						}
					}
				}
				color = color.operator/(numLights*4.0*SAMPLER);
				// Reflection
				if((depth-1) > 0 && (sceneMat->reflective.x + sceneMat->reflective.y + sceneMat->reflective.z > 0))
				{
					Vector reflection = (ray - normal.operator*(2*(ray.Dot(normal)))).Normalize();
					Vector refColor = trace(point, reflection, ref_index, (depth-1));
					color.x = color.x + sceneMat->reflective.x*refColor.x;
					color.y = color.y + sceneMat->reflective.y*refColor.y;
					color.z = color.z + sceneMat->reflective.z*refColor.z;
					//color = color.operator/(2);
				}
				// Refraction.
				if((depth-1) > 0 && (sceneMat->refraction.x + sceneMat->refraction.y + sceneMat->refraction.z > 0))
				{
					float n12 = ref_index/sceneMat->refraction_index;
					float sqroot = 1-n12*n12*(1 - ray.Dot(normal)*ray.Dot(normal));
					if(sqroot < 0)
						sqroot = 0;
					Vector refract_vector = (ray.operator*(n12) + normal.operator*(n12*(ray.Dot(normal) - sqrt(sqroot)))).Normalize();
					Vector refColor = trace(point, refract_vector, sceneMat->refraction_index, (depth-1));
					color.x = color.x + sceneMat->refraction.x*refColor.x;
					color.y = color.y + sceneMat->refraction.y*refColor.y;
					color.z = color.z + sceneMat->refraction.z*refColor.z;
				}
				//delete[] sceneMat;
				delete[] v;
			}
			else
				color = color + sceneBg.color;
		}
		return color;
	}

public:
	/* - Scene Variable for the Scene Definition - */
	Scene m_Scene;
	float win_width;
	float win_height;

	// -- Constructors & Destructors --
	RayTrace (void) {}
	~RayTrace (void) {}

	// -- Main Functions --
	void RenderScene(void)
	{
		if (&m_Scene)
		{
			// Get the Background
			SceneBackground sceneBg = m_Scene.GetBackground();
			glClearColor(sceneBg.color.x, sceneBg.color.y, sceneBg.color.z, 0);
			glLightModelfv(GL_LIGHT_MODEL_AMBIENT, (GLfloat *)&sceneBg.ambientLight);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Get the Camera
			Camera sceneCam = m_Scene.GetCamera();
			float ar = win_width/win_height;
			scr_height = 2*sceneCam.GetNearClip()*tan((PI*sceneCam.GetFOV())/360.0);
			Vector v = (sceneCam.target - sceneCam.position).Normalize();
			Vector normal_up = sceneCam.up.Cross(v).Normalize();
			Vector new_up = v.Cross(normal_up).Normalize();
			Vector w = new_up.Cross(v).Normalize();
			scr_width = ar*scr_height;

			glViewport(0, 0, win_width, win_height);
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			float scaled_width = scr_width;
			float scaled_height = scr_height;
			if(ar > 1)
				scaled_width = scr_width*ar;
			else
				scaled_height = scr_height/ar;
			gluOrtho2D(-scaled_width/2, scaled_width/2, -scaled_height/2, scaled_height/2);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			glBegin(GL_POINTS);
			{
				for (float y = 0.0; y < scaled_height; y = y + scaled_height/win_height)
				{
					for (float x = 0.0; x < scaled_width; x = x + scaled_width/win_width)
					{
						Vector P = sceneCam.position + v.operator*(sceneCam.GetNearClip()) + new_up.operator*(scaled_height/2 - y) + w.operator*(scaled_width/2 - x);
						Vector ray = (P - sceneCam.position).Normalize();
						Vector color = trace(sceneCam.position, ray, 1.0, m_Scene.GetDepth());
						glColor3f(color.x, color.y, color.z);
						glVertex2f(P.x, P.y);
					}
				}
			}
			glEnd();
		}
	}
};

#endif // RAYTRACE_H
