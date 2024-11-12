//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Maths.h"
#include "Matrix.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"
#include <execution>

#define PARALLEL_EXECUTION

using namespace dae;

Renderer::Renderer(SDL_Window * pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow))
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);
	m_AspectRatio = float(m_Width) / float(m_Height) ;
}

void Renderer::Render(Scene* pScene) const
{
	Camera& camera = pScene->GetCamera();
	const Matrix CameraToWorld = camera.CalculateCameraToWorld();

	const float aspectRatio = m_Width / static_cast<float>(m_Height);

	const float fovAngle = camera.fovAngle * TO_RADIANS;
	const float fov = tan(fovAngle / 2.f);

#if defined(PARALLEL_EXECUTION)
	//Parallel logic
	uint32_t amountOfPixels{ uint32_t(m_Width * m_Height) };
	std::vector<uint32_t> pixelIndices{};

	pixelIndices.reserve(amountOfPixels);
	for (uint32_t index{}; index < amountOfPixels; ++index) pixelIndices.emplace_back(index);

	std::for_each(std::execution::par, pixelIndices.begin(), pixelIndices.end(), [&](int i) {
		RenderPixel(pScene, i, fov, aspectRatio, CameraToWorld, camera.origin);
		});


#else 
	uint32_t AmountOfPixels{ uint32_t(m_Width * m_Height) };

	for (uint32_t pixelIndex{}; pixelIndex < AmountOfPixels; pixelIndex++)
	{
		RenderPixel(pScene, pixelIndex, fov, aspectRatio, CameraToWorld, camera.origin);
	}
#endif
	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

void Renderer::RenderPixel(Scene* pScene, uint32_t pixelIndex, float fov, float aspectRatio, const Matrix cameraToWorld, const Vector3 cameraOrigin) const
{
	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

	const uint32_t px{ pixelIndex % m_Width }, py{ pixelIndex / m_Width };

	float rayXValue{ px + 0.5f }, rayYValue{ py + 0.5f };
	float cx{ (2 * (rayXValue / float(m_Width)) - 1) * aspectRatio * fov };
	float cy{ (1 - (2 * (rayYValue / float(m_Height)))) * fov };

	float rayZValue{ 1.f };

	Vector3 rayDirection{ cx, cy, rayZValue };

	rayDirection = cameraToWorld.TransformVector(rayDirection).Normalized();

	Ray viewRay{ cameraOrigin , rayDirection };

	ColorRGB finalColor{};

	HitRecord closestHit{};

	pScene->GetClosestHit(viewRay, closestHit);

	if (closestHit.didHit)
	{
		const float occluderShadow{ 0.5 };
		Ray shadowRay{};
		shadowRay.origin = closestHit.origin;
		shadowRay.min = 0.001f;

		float lambertsLaw{};

		for (const Light& lightsIdx : lights)
		{

			shadowRay.max = LightUtils::GetDirectionToLight(lightsIdx, shadowRay.origin).Magnitude();
			shadowRay.direction = LightUtils::GetDirectionToLight(lightsIdx, shadowRay.origin) / shadowRay.max;


			if ((pScene->DoesHit(shadowRay) && m_ShadowsEnabled) == false)
			{
				lambertsLaw = Vector3::Dot(closestHit.normal, shadowRay.direction);
				if (lambertsLaw > 0)
				{
					switch (m_CurrentLightMode)
					{
					case dae::Renderer::LightingMode::ObservedArea:
						finalColor += ColorRGB{ lambertsLaw,  lambertsLaw , lambertsLaw };
						break;
					case dae::Renderer::LightingMode::Radiance:
						finalColor += LightUtils::GetRadiance(lightsIdx, shadowRay.origin);
						break;
					case dae::Renderer::LightingMode::BRDF:
						finalColor += materials[closestHit.materialIndex]->Shade(closestHit, -shadowRay.direction, rayDirection);
						break;
					case dae::Renderer::LightingMode::Combined:
						finalColor += (LightUtils::GetRadiance(lightsIdx, shadowRay.origin) * materials[closestHit.materialIndex]->Shade(closestHit, -shadowRay.direction, rayDirection) * lambertsLaw);
						break;
					}
				}
			}
		}
	}
	else finalColor = colors::Black;

	//Update Color in Buffer
	finalColor.MaxToOne();

	m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
		static_cast<uint8_t>(finalColor.r * 255),
		static_cast<uint8_t>(finalColor.g * 255),
		static_cast<uint8_t>(finalColor.b * 255));
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}

void dae::Renderer::CycleLightingMode()
{
	switch (m_CurrentLightMode)
	{
	case dae::Renderer::LightingMode::ObservedArea:
		m_CurrentLightMode = Renderer::LightingMode::Radiance;
		break;
	case dae::Renderer::LightingMode::Radiance:
		m_CurrentLightMode = Renderer::LightingMode::BRDF;
		break;
	case dae::Renderer::LightingMode::BRDF:
		m_CurrentLightMode = Renderer::LightingMode::Combined;
		break;
	case dae::Renderer::LightingMode::Combined:
		m_CurrentLightMode = Renderer::LightingMode::ObservedArea;
		break;
	}
}
