#pragma once
#include "Maths.h"

namespace dae
{
	namespace BRDF
	{
		/**
		 * \param kd Diffuse Reflection Coefficient
		 * \param cd Diffuse Color
		 * \return Lambert Diffuse Color
		 */
		static ColorRGB Lambert(float kd, const ColorRGB& cd)
		{
			//todo: W3
			ColorRGB rho{ kd  * cd };
			
			return { rho / M_PI };
		}

		static ColorRGB Lambert(const ColorRGB& kd, const ColorRGB& cd)
		{
			//todo: W3
			ColorRGB rho{ kd * cd };

			return { rho / M_PI };
		}

		/**
		 * \brief todo
		 * \param ks Specular Reflection Coefficient
		 * \param exp Phong Exponent
		 * \param l Incoming (incident) Light Direction
		 * \param v View Direction
		 * \param n Normal of the Surface
		 * \return Phong Specular Color
		 */
		static ColorRGB Phong(float ks, float exp, const Vector3& l, const Vector3& v, const Vector3& n)
		{
			//todo: W3
			const Vector3 reflection{(l - ((2 * std::max(Vector3::Dot(n,l), 0.01f))*n))};
			const float cosAngle{Vector3::Dot(reflection, v)};
			const float endResult{ ks * powf(cosAngle, exp)};

			const ColorRGB phongSpecular{ endResult , endResult , endResult };

			return { phongSpecular };
		}

		/**
		 * \brief BRDF Fresnel Function >> Schlick
		 * \param h Normalized Halfvector between View and Light directions
		 * \param v Normalized View direction
		 * \param f0 Base reflectivity of a surface based on IOR (Indices Of Refrection), this is different for Dielectrics (Non-Metal) and Conductors (Metal)
		 * \return
		 */
		static ColorRGB FresnelFunction_Schlick(const Vector3& h, const Vector3& v, const ColorRGB& f0)
		{
			//todo: W3
			const float TO_BE_POWERED{1 - (Vector3::Dot(h,v))};
			const ColorRGB ONE_MINUS{1 - f0.r, 1 - f0.g, 1 - f0.b};

			const ColorRGB END_RESULT{f0 + ONE_MINUS * powf(TO_BE_POWERED, 5)};

 			return { END_RESULT };
		}

		/**
		 * \brief BRDF NormalDistribution >> Trowbridge-Reitz GGX (UE4 implemetation - squared(roughness))
		 * \param n Surface normal
		 * \param h Normalized half vector
		 * \param roughness Roughness of the material
		 * \return BRDF Normal Distribution Term using Trowbridge-Reitz GGX
		 */
		static float NormalDistribution_GGX(const Vector3& n, const Vector3& h, float roughness)
		{
			//todo: W3
			const float a2{powf(roughness, 4)};
			const float DOT_POWER{powf(Vector3::Dot(n, h), 2)};
			
			const float END_RESULT{float(a2 / (M_PI * powf((DOT_POWER * (a2 - 1) + 1), 2)))};

			return { END_RESULT };
		}


		/**
		 * \brief BRDF Geometry Function >> Schlick GGX (Direct Lighting + UE4 implementation - squared(roughness))
		 * \param n Normal of the surface
		 * \param v Normalized view direction
		 * \param roughness Roughness of the material
		 * \return BRDF Geometry Term using SchlickGGX
		 */
		static float GeometryFunction_SchlickGGX(const Vector3& n, const Vector3& v, float roughness)
		{
			//todo: W3
			const float a{ powf(roughness, 2) };
			const float DOT { std::max(Vector3::Dot(n, v), 0.01f) };
			const float K { powf((a + 1) , 2) / 8};

			float DENOMINATOR {(DOT * (1 - K)) + K};
			const float END_RESULT{DOT / DENOMINATOR};

			return { END_RESULT };
		}

		/**
		 * \brief BRDF Geometry Function >> Smith (Direct Lighting)
		 * \param n Normal of the surface
		 * \param v Normalized view direction
		 * \param l Normalized light direction
		 * \param roughness Roughness of the material
		 * \return BRDF Geometry Term using Smith (> SchlickGGX(n,v,roughness) * SchlickGGX(n,l,roughness))
		 */
		static float GeometryFunction_Smith(const Vector3& n, const Vector3& v, const Vector3& l, float roughness)
		{
			//todo: W3
			const float SHADOWING{GeometryFunction_SchlickGGX(n, v, roughness)};
			const float MASKING{GeometryFunction_SchlickGGX(n, l, roughness)};

			return { SHADOWING * MASKING};
		}

	}
}