#include "pch.h"
#include "TMEffectFirework.h"
#include "TMCamera.h"
#include "TMGlobal.h"
#include "TMUtil.h"

namespace
{
constexpr float PI = 3.1415927f;
constexpr float FIREWORK_LIFETIME = 10.0f;
constexpr unsigned int FIREWORK_LIFETIME_MS = 10000;
constexpr float FIREWORK_LAUNCH_MS = 3000.0f;

float RandomSigned()
{
	return static_cast<float>(rand() - rand()) / static_cast<float>(RAND_MAX);
}

TMVector3 Add(const TMVector3& left, const TMVector3& right)
{
	return { left.x + right.x, left.y + right.y, left.z + right.z };
}

TMVector3 Scale(const TMVector3& vector, float scale)
{
	return { vector.x * scale, vector.y * scale, vector.z * scale };
}
}

TMEffectFireWork::TMEffectFireWork(TMVector3 vecPositiont, int nType) : TreeNode(0)
{
	m_nType = nType;
	m_dwColor = 0;
	m_dwStartTime = g_pTimerManager->GetServerTime();
	m_bBomb = 0;
	m_nColorType = -1;

	for (int i = 0; i < 100; ++i)
	{
		SetParticle(
			&m_Particle[i],
			static_cast<unsigned short>(i % 3),
			FIREWORK_LIFETIME,
			vecPositiont,
			{ 0.0f, 5.0f, 0.0f });
		m_vAddFireWork[i] = {};
		m_ArrCustomShape[i] = 0;
	}

	m_vertex[0].position = { -0.5f, -0.5f, 0.0f };
	m_vertex[1].position = { 0.5f, -0.5f, 0.0f };
	m_vertex[2].position = { 0.5f, 0.5f, 0.0f };
	m_vertex[3].position = { -0.5f, 0.5f, 0.0f };

	m_vertex[0].tu = 0.02f;
	m_vertex[0].tv = 0.98f;
	m_vertex[1].tu = 0.98f;
	m_vertex[1].tv = 0.98f;
	m_vertex[2].tu = 0.98f;
	m_vertex[2].tv = 0.02f;
	m_vertex[3].tu = 0.02f;
	m_vertex[3].tv = 0.02f;

	GetSoundAndPlay(315, 0, 0);
}

TMEffectFireWork::~TMEffectFireWork()
{
}

int TMEffectFireWork::FrameMove(unsigned int dwServerTime)
{
	dwServerTime = g_pTimerManager->GetServerTime();
	const unsigned int elapsed = dwServerTime - m_dwStartTime;
	if (elapsed > FIREWORK_LIFETIME_MS)
	{
		g_pObjectManager->DeleteObject(this);
		return 1;
	}

	const float time = static_cast<float>(elapsed) / FIREWORK_LAUNCH_MS - 1.0f;
	const unsigned int colorElapsed = (std::min)(elapsed, 7000u);
	const int intensity = static_cast<int>(
		sinf(static_cast<float>(colorElapsed) / 7000.0f * PI) * 255.0f);

	int red = 0;
	int green = 0;
	int blue = 0;
	const int colorType = m_nType == 6 ? m_nColorType : m_nType;
	switch (colorType)
	{
	case 0:
		red = green = intensity;
		break;
	case 1:
		green = intensity;
		break;
	case 2:
		red = blue = intensity;
		break;
	case 3:
		red = green = blue = intensity;
		break;
	case 4:
		red = intensity;
		break;
	case 5:
		green = blue = intensity;
		break;
	}
	m_dwColor = (static_cast<unsigned int>(red) << 16)
		| (static_cast<unsigned int>(green) << 8)
		| static_cast<unsigned int>(blue);

	for (int i = 0; i < 100; ++i)
	{
		auto& particle = m_Particle[i];
		if (time >= 0.0f)
		{
			if (!m_bBomb)
			{
				m_bBomb = 1;
				GetSoundAndPlay(316, 0, 0);
			}

			if (m_nType < 6)
			{
				constexpr float damping = 1.8f;
				const TMVector3 gravity{ 0.0f, -9.8f, 0.0f };
				const float curve = ((1.0f - expf(-damping * time)) * 0.5f)
					/ (damping * damping);
				const TMVector3 velocityAndGravity = Add(
					Scale(particle.vInitialVelocity, damping),
					gravity);
				particle.vPosition = Add(
					Add(particle.vInitialPosition, Scale(gravity, time / damping)),
					Scale(velocityAndGravity, curve));
			}
			else if (m_nType == 6)
			{
				float scale = 0.5f;
				if (time < 0.5f)
					scale = sinf(time * PI) * 0.5f;

				particle.vPosition.x = particle.vInitialPosition.x + m_vAddFireWork[i].x * scale;
				particle.vPosition.z = particle.vInitialPosition.z + m_vAddFireWork[i].z * scale;
				particle.vPosition.y = particle.vInitialPosition.y + m_vAddFireWork[i].y * scale
					- sinf(time * PI / 4.0f) * time * 3.0f;
			}

			particle.fMaturity = time / particle.fLifetime;
			const float sizeTime = particle.fMaturity + 0.5f;
			particle.fSize = expf(-(sizeTime * sizeTime)) * 0.3f;
		}
		else
		{
			const float delay = (RandomSigned() + 1.0f) / 20.0f;
			particle.vPosition = Add(
				particle.vInitialPosition,
				Scale(particle.vLaunchVelocity, 1.5f * (time - delay)));
		}
	}

	return 1;
}

int TMEffectFireWork::Render()
{
	if (m_nType != 6 && g_bHideEffect == 1)
		return 1;
	if (!g_pDevice || !g_pDevice->m_pd3dDevice || !g_pTextureManager
		|| !g_pObjectManager || !g_pObjectManager->m_pCamera)
		return 0;

	auto pCamera = g_pObjectManager->m_pCamera;
	D3DXMATRIX rotation{};
	D3DXMatrixRotationYawPitchRoll(
		&rotation,
		PI * 0.5f - pCamera->m_fHorizonAngle,
		-pCamera->m_fVerticalAngle,
		0.0f);

	g_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, 1u);
	g_pDevice->SetRenderState(D3DRS_DESTBLEND, 2u);
	g_pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, 4u);
	g_pDevice->SetRenderState(D3DRS_FOGENABLE, 0);
	g_pDevice->SetRenderState(D3DRS_LIGHTING, 0);
	g_pDevice->SetRenderState(D3DRS_SRCBLEND, 5u);
	g_pDevice->SetRenderState(D3DRS_ALPHAFUNC, 8u);
	g_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, 0);
	g_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, 0);
	g_pDevice->SetRenderState(D3DRS_CULLMODE, 1u);
	g_pDevice->m_pd3dDevice->SetFVF(322u);
	g_pDevice->SetTexture(0, g_pTextureManager->GetEffectTexture(7, 360000));

	for (int i = 0; i < 100; ++i)
	{
		const auto& particle = m_Particle[i];
		if (particle.fMaturity >= 1.0f)
			continue;

		D3DXMATRIX scale{};
		D3DXMATRIX translation{};
		D3DXMATRIX world{};
		D3DXMatrixScaling(&scale, particle.fSize, particle.fSize, particle.fSize);
		D3DXMatrixTranslation(
			&translation,
			particle.vPosition.x,
			particle.vPosition.y,
			particle.vPosition.z);
		world = rotation * scale * translation;
		g_pDevice->m_pd3dDevice->SetTransform(D3DTS_WORLD, &world);

		for (auto& vertex : m_vertex)
			vertex.diffuse = m_dwColor;
		g_pDevice->m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2u, m_vertex, sizeof(RDLVERTEX));

		D3DXMatrixScaling(
			&scale,
			particle.fSize * 0.7f,
			particle.fSize * 0.7f,
			particle.fSize * 0.7f);
		world = rotation * scale * translation;
		g_pDevice->m_pd3dDevice->SetTransform(D3DTS_WORLD, &world);
		for (auto& vertex : m_vertex)
			vertex.diffuse = 0xAAAAAAAA;
		g_pDevice->m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2u, m_vertex, sizeof(RDLVERTEX));
	}

	g_pDevice->SetRenderState(D3DRS_LIGHTING, 1u);
	g_pDevice->SetRenderState(D3DRS_SRCBLEND, 2u);
	g_pDevice->SetRenderState(D3DRS_ALPHAFUNC, 7u);
	g_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, 1u);
	g_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, 1u);
	g_pDevice->SetRenderState(D3DRS_CULLMODE, 3u);
	return 1;
}

Particle* TMEffectFireWork::SetParticle(Particle* result, unsigned short wType, float fLifeTime, TMVector3 vBasePosition, TMVector3 vBaseVelocity)
{
	if (!result)
		return nullptr;

	TMVector3 direction{ RandomSigned(), RandomSigned(), RandomSigned() };
	const float length = direction.Length();
	if (length > 0.0f)
		direction = Scale(direction, 15.0f / length);

	result->vPosition = {};
	result->vLaunchVelocity = vBaseVelocity;
	result->vInitialPosition = vBasePosition;
	result->vInitialVelocity = direction;
	result->fLifetime = (RandomSigned() / 4.0f + 1.0f) * fLifeTime;
	result->fMaturity = 0.0f;
	result->wType = wType;
	result->fSize = 0.1f;
	return result;
}

void TMEffectFireWork::SetCustomFireWork(const char* pBuffer)
{
	if (m_nType != 6)
		return;

	for (int i = 0; i < 100; ++i)
	{
		const auto byte = pBuffer ? static_cast<unsigned char>(pBuffer[i / 8]) : 0;
		m_ArrCustomShape[i] = static_cast<char>((byte >> (i % 8)) & 1u);
	}

	int activePixels = 0;
	for (char pixel : m_ArrCustomShape)
	{
		if (pixel)
			++activePixels;
	}
	if (activePixels == 0)
	{
		m_ArrCustomShape[0] = 1;
		activePixels = 1;
	}

	const int copies = 100 / activePixels;
	const float angle = -PI / 4.0f;
	const float sine = sinf(angle);
	const float cosine = cosf(angle);
	int particleIndex = 0;
	int lastRow = 0;
	int lastColumn = 0;

	for (int row = 0; row < 10 && particleIndex < 100; ++row)
	{
		for (int column = 0; column < 10 && particleIndex < 100; ++column)
		{
			if (!m_ArrCustomShape[row * 10 + column])
				continue;

			const float gridX = static_cast<float>(5 - row) * 0.3f;
			const float gridZ = static_cast<float>(4 - column) * 0.3f;
			lastRow = row;
			lastColumn = column;

			for (int copy = 0; copy < copies && particleIndex < 100; ++copy)
			{
				if (copy == 0)
				{
					m_vAddFireWork[particleIndex] = {
						cosine * gridZ + sine * gridX,
						-1.5f,
						sine * gridZ - cosine * gridX
					};
				}
				else
				{
					m_vAddFireWork[particleIndex] = {
						gridX + RandomSigned() / 0.16f,
						RandomSigned() / 0.08f * 0.3f - 1.5f,
						gridZ + RandomSigned() / 0.16f
					};
				}
				++particleIndex;
			}
		}
	}

	while (particleIndex < 100)
	{
		m_vAddFireWork[particleIndex] = {
			static_cast<float>(lastRow) * 0.3f + RandomSigned() / 0.08f,
			-1.5f,
			static_cast<float>(lastColumn) * 0.3f + RandomSigned() / 0.08f
		};
		++particleIndex;
	}

	m_nColorType = rand() % 6;
}
