#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/CinderImGui.h"
#include "cinder/Log.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class DockingUIApp : public App {
  public:
	static void prepareSettings(Settings* settings);
	void setup() override;
	void keyDown( KeyEvent event ) override;
	void update() override;
	void drawSceneInFbo();
	void drawPrimaryWindow();
	void drawUI();

	float mUIScale = 1.0;
	float mCircleRadius = 50.0;

	glm::ivec2 mSceneResolution = glm::ivec2(1024, 1024);
	int mSceneMSAA = 0;
	gl::FboRef mSceneFbo;

	bool mMaximizeScene = false;
};

void DockingUIApp::prepareSettings(Settings* settings) {
	auto display = Display::getMainDisplay();
	auto size = glm::vec2(display->getSize()) * 0.9f;
	auto pos = glm::vec2(display->getSize()) * 0.05f;
	settings->setWindowSize(size);
	settings->setWindowPos(pos);
}

void DockingUIApp::setup() {
	// the usual draw() is called for each window. we only want to draw in our primary window so instead we connect the windows draw signal to a specific draw function
	getWindow()->getSignalDraw().connect(std::bind(&DockingUIApp::drawPrimaryWindow, this));

	// initialize Dear ImGui and enable the viewports feature
	ImGui::Initialize(ImGui::Options().enableViewports(true));
}

void DockingUIApp::keyDown( KeyEvent event ) {
	if (event.getCode() == KeyEvent::KEY_f) {
		mMaximizeScene = !mMaximizeScene;
	}
}

void DockingUIApp::update() {
	drawSceneInFbo();
}

void DockingUIApp::drawSceneInFbo() {
	// create FBO if it hasn't been created yet or if the resolution is wrong
	if (!mSceneFbo || mSceneFbo->getSize() != mSceneResolution || mSceneFbo->getFormat().getSamples() != mSceneMSAA) {
		mSceneFbo = gl::Fbo::create(mSceneResolution.x, mSceneResolution.y, gl::Fbo::Format().samples(mSceneMSAA));
	}

	gl::ScopedFramebuffer scpFbo(mSceneFbo);
	gl::ScopedViewport scpVp(ivec2(0), mSceneFbo->getSize());
	gl::ScopedMatrices scpMtx;
	gl::setMatricesWindow(mSceneFbo->getSize());

	// do your drawing here
	gl::clear(Color(0, 0, 0));
	gl::drawSolidCircle(mSceneResolution / 2, mCircleRadius);
}

void DockingUIApp::drawPrimaryWindow() {
	if (mMaximizeScene && mSceneFbo) {
		gl::clear(Color(0, 0, 0));
		auto tex = mSceneFbo->getColorTexture();
		Rectf srcRect = tex->getBounds();
		Rectf dstRect = getWindowBounds();
		Rectf fitRect = srcRect.getCenteredFit(dstRect, true);
		gl::draw(tex, fitRect);
	}
	else {
		drawUI();
	}
}

void DockingUIApp::drawUI() {
	// set up a fullscreen dockspace for our windows to dock in
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->GetWorkPos());
		ImGui::SetNextWindowSize(viewport->GetWorkSize());
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::Begin("Docking", 0, windowFlags);
		ImGui::DockSpace(ImGui::GetID("DockSpace"));
		ImGui::End();
		ImGui::PopStyleVar(2);
	}

	// draw our scene in a window
	{
		ImGui::Begin("Scene");
		if (mSceneFbo) {
			auto tex = mSceneFbo->getColorTexture();
			// fit in window
			auto contentRegionMin = ImGui::GetWindowContentRegionMin();
			auto contentRegionMax = ImGui::GetWindowContentRegionMax();
			auto srcRect = Rectf(0, 0, tex->getWidth(), tex->getHeight());
			auto dstRect = Rectf(contentRegionMin.x, contentRegionMin.y, contentRegionMax.x, contentRegionMax.y);
			auto fitRect = srcRect.getCenteredFit(dstRect, true);
			glm::vec2 scale = fitRect.getSize() / srcRect.getSize();
			glm::vec2 size = srcRect.getSize() * scale;
			glm::vec2 offset = contentRegionMin;
			offset += (dstRect.getSize() - size) / 2.0f;
			ImGui::SetCursorPos(offset);
			ImGui::Image(tex, size);
		}
		ImGui::End();
	}

	// draw some GUI windows
	{
		ImGui::Begin("Window 1");
		ImGui::SetWindowFontScale(mUIScale);
		ImGui::DragInt2("Scene resolution", &mSceneResolution[0], 1.0, 1, 4096);
		ImGui::DragInt("Scene MSAA", &mSceneMSAA, 1.0f, 0, 32);
		ImGui::End();
	}
	{
		ImGui::Begin("Window 2");
		ImGui::SetWindowFontScale(mUIScale);
		ImGui::DragFloat("Circle radius", &mCircleRadius);
		ImGui::End();
	}
	{
		ImGui::Begin("Window 3");
		ImGui::SetWindowFontScale(mUIScale);
		ImGui::DragFloat("UI scale", &mUIScale, 0.1, 0.25, 10.0);
		ImGui::End();
	}
	{
		ImGui::Begin("Window 4");
		ImGui::SetWindowFontScale(mUIScale);
		static int comboIndex = 0;
		static vector<string> comboValues = { "One", "Two", "Three" };
		ImGui::Combo("Combo box", &comboIndex, comboValues, ImGuiComboFlags_None);
		ImGui::End();
	}

}

// if msaa is disabled, all our gui windows flicker. haven't figured this one out yet
CINDER_APP( DockingUIApp, RendererGl(RendererGl::Options().msaa(2)), DockingUIApp::prepareSettings )
