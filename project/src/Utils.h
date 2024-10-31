#pragma once
#include <fstream>
#include "Maths.h"
#include "DataTypes.h"

namespace dae
{
	namespace GeometryUtils
	{
#pragma region Sphere HitTest
		//SPHERE HIT-TESTS
		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W1
			float A = Vector3::Dot(ray.direction, ray.direction);
			float B = Vector3::Dot((2 * ray.direction), (ray.origin - sphere.origin));
			float C = Vector3::Dot((ray.origin - sphere.origin), (ray.origin - sphere.origin)) - (sphere.radius * sphere.radius);
			float t{};

			float Discriminant{(B*B) - (4*A*C)};

			if (Discriminant > 0)
			{
				float sqrtDiscriminant{ sqrt(Discriminant) };
				float denominator{ 2 * A };
					

				t = (-B - sqrtDiscriminant) / denominator;

				if (t < ray.min)
				{
					t = (-B + sqrtDiscriminant) / denominator;
				}

				if (t < ray.min || t > ray.max) return false;

				if (ignoreHitRecord == false)
				{
					hitRecord.didHit = true;
					hitRecord.t = t;
					hitRecord.origin = ray.origin + (t * ray.direction);
					Vector3 unnormalizedVector{ hitRecord.origin - sphere.origin };
					hitRecord.normal = unnormalizedVector.Normalized();
					hitRecord.materialIndex = sphere.materialIndex;
				}
	
				return true;
				
			}
			else return false;

		}

		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Sphere(sphere, ray, temp, true);
		}
#pragma endregion
#pragma region Plane HitTest
		//PLANE HIT-TESTS
		inline bool HitTest_Plane(const Plane& plane, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W1
			float numerator{Vector3::Dot((plane.origin - ray.origin), plane.normal)};
			float denominator{Vector3::Dot(ray.direction, plane.normal)};
			float t{ numerator / denominator };

			if( t > ray.min && t < ray.max)
			{
				if(ignoreHitRecord == false)
				{
					hitRecord.t = t;
					hitRecord.didHit = true;
					hitRecord.origin = ray.origin + (t * ray.direction);
					/*if(Vector3::Dot(hitRecord.origin, ray.origin) >= 0) hitRecord.normal = plane.normal;
					else hitRecord.normal = -plane.normal;*/
					hitRecord.normal = plane.normal;
					hitRecord.materialIndex = plane.materialIndex;
				}

				return true;
			}
			else return false;
		}

		inline bool HitTest_Plane(const Plane& plane, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Plane(plane, ray, temp, true);
		}
#pragma endregion
#pragma region Triangle HitTest
		//TRIANGLE HIT-TESTS
		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W5
			// Clockwise Edge
			Vector3 n{triangle.normal};
			/*if (n == Vector3{ 0, 0, 0 })
			{
				Vector3 a{ triangle.v1 - triangle.v0 };

				Vector3 b{ triangle.v2 - triangle.v0 };
				n = Vector3::Cross(a, b).Vector3::Normalized();
			}*/
			
			float nDotv{ Vector3::Dot(ray.direction, n) };

			if (nDotv == 0) return false;

			if(ignoreHitRecord == false)
			{
				if (triangle.cullMode == TriangleCullMode::BackFaceCulling && nDotv > 0) return false;
				else if (triangle.cullMode == TriangleCullMode::FrontFaceCulling && nDotv < 0) return false;
			}
			else
			{
				if (triangle.cullMode == TriangleCullMode::BackFaceCulling && nDotv < 0) return false;
				else if (triangle.cullMode == TriangleCullMode::FrontFaceCulling && nDotv > 0) return false;
			}

			float numerator{ Vector3::Dot((triangle.v0 - ray.origin), n) };
			float denominator{ Vector3::Dot(ray.direction, n) };
			float t{ numerator / denominator };

			if (t < ray.min || t > ray.max) return false;

			Vector3 p{ ray.origin + (ray.direction * t) };
			Vector3 triangleNode[3]{triangle.v0 , triangle.v1, triangle.v2};

			for(int edgeIdx{}; edgeIdx < 3; ++edgeIdx)
			{
				Vector3 e{ triangleNode[edgeIdx + 1] - triangleNode[edgeIdx]};
				if (edgeIdx == 2) e = triangleNode[0] - triangleNode[2];
				Vector3 pV{ p - triangleNode[edgeIdx] };

				if (Vector3::Dot(Vector3::Cross(e, pV), n) < 0) return false;
			}

			if (ignoreHitRecord == false)
			{
				hitRecord.t = t;
				hitRecord.didHit = true;
				hitRecord.origin = ray.origin + (t * ray.direction);
				hitRecord.normal = n;
				hitRecord.materialIndex = triangle.materialIndex;
			}

			return true;
		}

		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Triangle(triangle, ray, temp, true);
		}
#pragma endregion
#pragma region TriangeMesh HitTest
		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W5

			HitRecord smallestHit{};
			smallestHit.t = hitRecord.t;

			for(int triangleIdx{}; triangleIdx < (mesh.indices.size() / 3); ++triangleIdx)
			{
				Triangle polygon{ mesh.transformedPositions[mesh.indices[triangleIdx * 3]],
				mesh.transformedPositions[mesh.indices[triangleIdx * 3 + 1]],
				mesh.transformedPositions[mesh.indices[triangleIdx * 3 + 2]],
				mesh.transformedNormals[triangleIdx]};
				polygon.cullMode = mesh.cullMode;
				polygon.materialIndex = mesh.materialIndex;

				HitTest_Triangle(polygon, ray, hitRecord);

				if (hitRecord.t < smallestHit.t)
				{
					smallestHit = hitRecord;
				}
			}


			hitRecord = smallestHit;

			if(hitRecord.didHit) return true;
			return false;
		}

		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_TriangleMesh(mesh, ray, temp, true);
		}
#pragma endregion
	}

	namespace LightUtils
	{
		//Direction from target to light
		inline Vector3 GetDirectionToLight(const Light& light, const Vector3 origin)
		{
			//todo W3
			Vector3 unnormalizedVector{};

			if(light.type == LightType::Directional)
			{
				unnormalizedVector = -(light.direction) * FLT_MAX;
			}
			else if (light.type == LightType::Point)
			{
				unnormalizedVector = light.origin - origin;
			}

			return unnormalizedVector;
		}

		inline ColorRGB GetRadiance(const Light& light, const Vector3& target)
		{
			//todo W3
			ColorRGB Ergb{};

			if (light.type == LightType::Directional)
			{
				Ergb = light.color * light.intensity;
			}
			else if (light.type == LightType::Point)
			{
				Vector3 r{ light.origin - target };
				float Dot{ Vector3::Dot(r, r) };

				if (Dot == 0.f) Dot = 0.0001f;

				Ergb = light.color * (light.intensity / Dot);
			}

			return Ergb;
		}
	}

	namespace Utils
	{
		//Just parses vertices and indices
#pragma warning(push)
#pragma warning(disable : 4505) //Warning unreferenced local function
		static bool ParseOBJ(const std::string& filename, std::vector<Vector3>& positions, std::vector<Vector3>& normals, std::vector<int>& indices)
		{
			std::ifstream file(filename);
			if (!file)
				return false;

			std::string sCommand;
			// start a while iteration ending when the end of file is reached (ios::eof)
			while (!file.eof())
			{
				//read the first word of the string, use the >> operator (istream::operator>>) 
				file >> sCommand;
				//use conditional statements to process the different commands	
				if (sCommand == "#")
				{
					// Ignore Comment
				}
				else if (sCommand == "v")
				{
					//Vertex
					float x, y, z;
					file >> x >> y >> z;
					positions.push_back({ x, y, z });
				}
				else if (sCommand == "f")
				{
					float i0, i1, i2;
					file >> i0 >> i1 >> i2;

					indices.push_back((int)i0 - 1);
					indices.push_back((int)i1 - 1);
					indices.push_back((int)i2 - 1);
				}
				//read till end of line and ignore all remaining chars
				file.ignore(1000, '\n');

				if (file.eof())
					break;
			}

			//Precompute normals
			for (uint64_t index = 0; index < indices.size(); index += 3)
			{
				uint32_t i0 = indices[index];
				uint32_t i1 = indices[index + 1];
				uint32_t i2 = indices[index + 2];

				Vector3 edgeV0V1 = positions[i1] - positions[i0];
				Vector3 edgeV0V2 = positions[i2] - positions[i0];
				Vector3 normal = Vector3::Cross(edgeV0V1, edgeV0V2);

				if (std::isnan(normal.x))
				{
					int k = 0;
				}

				normal.Normalize();
				if (std::isnan(normal.x))
				{
					int k = 0;
				}

				normals.push_back(normal);
			}

			return true;
		}
#pragma warning(pop)
	}
}