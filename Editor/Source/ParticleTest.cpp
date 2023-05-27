#include <random>
#include <SnowEngine.h>

struct Particle {
	glm::vec2 Position;
	glm::vec2 Velocity;
	glm::vec4 Color;
};

class ParticleTest
{
public:
	ParticleTest();
	~ParticleTest();

	void Run();

private:
	void InitStorageBuffers();
	void RunCompute(f32 deltaTime) const;
	void RunGraphics(f32 deltaTime) const;

	std::shared_ptr<SnowEngine::Window> mWindow;
	std::shared_ptr<SnowEngine::Surface> mSurface;
	std::shared_ptr<SnowEngine::RenderPass> mRenderPass;
	std::shared_ptr<SnowEngine::Shader> mShader;
	std::shared_ptr<SnowEngine::Shader> mComputeShader;
	std::shared_ptr<SnowEngine::Pipeline> mPipeline;
	std::shared_ptr<SnowEngine::ComputePipeline> mComputePipeline;
	std::shared_ptr<SnowEngine::CommandBuffer> mRenderCmd;
	std::shared_ptr<SnowEngine::CommandBuffer> mComputeCmd;
	std::shared_ptr<SnowEngine::DescriptorSet> mComputeDescriptorSet;
	std::shared_ptr<SnowEngine::StorageBuffer> mParticleBufferIn;
	std::shared_ptr<SnowEngine::StorageBuffer> mParticleBufferOut;
	u32 mParticleCount;
};

ParticleTest::ParticleTest()
{
	SnowEngine::GraphicsCore::Init();

	mWindow = SnowEngine::Window::Create("Particle Test", 1920, 1080);
	mSurface = SnowEngine::Surface::Create(mWindow);
	mRenderPass = SnowEngine::RenderPass::Create(mSurface, false);

	mShader = SnowEngine::Shader::Create(
	{
		{ "D:/Dev/SnowEngine/Engine/Resources/Shaders/default.vert", SnowEngine::ShaderType::Vertex },
		{ "D:/Dev/SnowEngine/Engine/Resources/Shaders/default.frag", SnowEngine::ShaderType::Fragment },
		{}
	}, "default");
	mComputeShader = SnowEngine::Shader::Create(SnowEngine::ComputeShaderSource
	{
		{ "D:/Dev/SnowEngine/Engine/Resources/Shaders/particle.comp", SnowEngine::ShaderType::Compute }
	}, "particle");
	mPipeline = SnowEngine::Pipeline::Create({ mShader, mRenderPass, 1920, 1080 });
	mComputePipeline = SnowEngine::ComputePipeline::Create(mComputeShader);

	mRenderCmd = SnowEngine::CommandBuffer::Create(mSurface->ImageCount(), SnowEngine::CommandBufferUsage::Graphics);
	mComputeCmd = SnowEngine::CommandBuffer::Create(mSurface->ImageCount(), SnowEngine::CommandBufferUsage::Compute);

	mComputeDescriptorSet = SnowEngine::DescriptorSet::Create(mComputeShader, 0, mSurface->ImageCount());

	InitStorageBuffers();
}

ParticleTest::~ParticleTest()
{

}

void ParticleTest::Run()
{
	auto currentTime = std::chrono::high_resolution_clock::now();
	auto lastTime = std::chrono::high_resolution_clock::now();
	while(!mWindow->Closing())
	{
		currentTime = std::chrono::high_resolution_clock::now();
		const f32 time = std::chrono::duration<f32, std::chrono::seconds::period>(currentTime - lastTime).count() * 1000.0f;

		mSurface->Begin();

		RunCompute(time);
		RunGraphics(time);

		mSurface->End(mRenderCmd);

		mParticleBufferIn->SetData(mParticleBufferOut);

		SnowEngine::Window::Update();

		lastTime = currentTime;
	}
}

void ParticleTest::InitStorageBuffers()
{
	mParticleCount = 8192;

	std::default_random_engine rndEngine(static_cast<u32>(time(nullptr)));
	std::uniform_real_distribution<f32> rndDist(0.0f, 1.0f);

	std::vector<Particle> particles(mParticleCount);
	for (auto& [position, velocity, color] : particles) {
		const f32 r = 0.25f * sqrt(rndDist(rndEngine));
		const f32 theta = rndDist(rndEngine) * 2 * 3.14159265358979323846;
		const f32 x = r * cos(theta) * 2560 / 1440;
		const f32 y = r * sin(theta);

		position = glm::vec2(x, y);
		velocity = glm::normalize(glm::vec2(x, y)) * 0.00025f;
		color = glm::vec4(rndDist(rndEngine), rndDist(rndEngine), rndDist(rndEngine), 1.0f);
	}

	mParticleBufferIn = SnowEngine::StorageBuffer::Create(particles.size() * sizeof(Particle));
	mParticleBufferOut = SnowEngine::StorageBuffer::Create(particles.size() * sizeof(Particle));
	mParticleBufferIn->SetData(particles.data());

	mComputeDescriptorSet->SetStorageBuffer("ParticleSSBOIn", mParticleBufferIn);
	mComputeDescriptorSet->SetStorageBuffer("ParticleSSBOOut", mParticleBufferOut);
}

void ParticleTest::RunCompute(const f32 deltaTime) const
{
	mComputeCmd->Begin(mSurface->CurrentFrame());

	mComputeDescriptorSet->SetUniform("ParameterUBO", &deltaTime, mSurface->CurrentFrame());
	mComputePipeline->BindDescriptorSet(mComputeDescriptorSet.get(), mSurface->CurrentFrame(), mComputeCmd);
	mComputePipeline->Dispatch(mParticleCount / 256, 1, 1, mComputeCmd);

	mComputeCmd->End(mSurface->CurrentFrame());

	mComputeCmd->Submit(mSurface->CurrentFrame(), mSurface);
}

void ParticleTest::RunGraphics(const f32 deltaTime) const
{
	mRenderCmd->Begin(mSurface->CurrentFrame());
	mRenderPass->Begin(mRenderCmd);

	mPipeline->Bind(mRenderCmd);

	//TODO: draw outBuffer

	mRenderPass->End(mRenderCmd);
	mRenderCmd->End(mSurface->CurrentFrame());
	mRenderCmd->Submit(mSurface->CurrentFrame(), mComputeCmd);
}

int notMain()
{
	ParticleTest test{};
	test.Run();

	return 0;
}
