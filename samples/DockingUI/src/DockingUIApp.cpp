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
	ImGuiStyle setupStyle();
	void keyDown( KeyEvent event ) override;
	void update() override;
	void drawSceneInFbo();
	void drawPrimaryWindow();
	void drawUI();

	float mUIScale = 1.0;
	float mCircleRadius = 50.0;
	std::string mInputString = "";

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
	auto style = setupStyle();
	auto opts = ImGui::Options().style(style).enableViewports(true);
	ImGui::Initialize(opts);
}

ImGuiStyle DockingUIApp::setupStyle() {
	ImGuiStyle style;
	
	// solarized https://en.wikipedia.org/wiki/Solarized_(color_scheme)
	ImVec4 Base035{ 3 / 255.0f, 35 / 255.0f, 44 / 255.0f, 1.0f };	// made up
	ImVec4 Base03{ 0  / 255.0f,	 43  / 255.0f, 54  / 255.0f, 1.0f };
	ImVec4 Base02{ 7  / 255.0f,	 54  / 255.0f, 66  / 255.0f, 1.0f };
	ImVec4 Base015{ 51 / 255.0f, 69 / 255.0f, 75 / 255.0f, 1.0f };	// made up
	ImVec4 Base01{ 88  / 255.0f, 110 / 255.0f, 117 / 255.0f, 1.0f };
	ImVec4 Base00{ 101 / 255.0f, 123 / 255.0f, 131 / 255.0f, 1.0f };
	ImVec4 Base0{ 131 / 255.0f, 148 / 255.0f, 150 / 255.0f, 1.0f };
	ImVec4 Base1{ 147 / 255.0f, 161 / 255.0f, 161 / 255.0f, 1.0f };
	ImVec4 Base2{ 238 / 255.0f, 232 / 255.0f, 213 / 255.0f, 1.0f };
	ImVec4 Base3{ 253 / 255.0f, 246 / 255.0f, 227 / 255.0f, 1.0f };
	ImVec4 Yellow{ 181 / 255.0f, 137 / 255.0f, 0   / 255.0f, 1.0f };
	ImVec4 Orange{ 203 / 255.0f, 75  / 255.0f, 22  / 255.0f, 1.0f };
	ImVec4 Red{ 220 / 255.0f, 50 / 255.0f, 47 / 255.0f, 1.0f };
	ImVec4 Magenta{ 211 / 255.0f, 54 / 255.0f, 130 / 255.0f, 1.0f };
	ImVec4 Violet{ 108 / 255.0f, 113 / 255.0f, 196 / 255.0f, 1.0f };
	ImVec4 Blue{ 38  / 255.0f, 139 / 255.0f, 210 / 255.0f, 1.0f };
	ImVec4 Cyan{ 42  / 255.0f, 161 / 255.0f, 152 / 255.0f, 1.0f };
	ImVec4 Green{ 133 / 255.0f, 153 / 255.0f, 0   / 255.0f, 1.0f };

	


	style.Colors[ImGuiCol_Text] = Base1;
	style.Colors[ImGuiCol_TextDisabled] = Base1;
	style.Colors[ImGuiCol_WindowBg] = Base03;
	style.Colors[ImGuiCol_ChildBg] = Base03;
	style.Colors[ImGuiCol_PopupBg] = Base03;
	style.Colors[ImGuiCol_Border] = Base02;
	style.Colors[ImGuiCol_BorderShadow] = Base02;
	style.Colors[ImGuiCol_FrameBg] = Base02;
	style.Colors[ImGuiCol_FrameBgHovered] = Base015;
	style.Colors[ImGuiCol_FrameBgActive] = Base035;
	style.Colors[ImGuiCol_TitleBg] = Base02;
	style.Colors[ImGuiCol_TitleBgActive] = Base015;
	style.Colors[ImGuiCol_TitleBgCollapsed] = Base035;
	style.Colors[ImGuiCol_Tab] = Base02;
	style.Colors[ImGuiCol_TabHovered] = Base01;
	style.Colors[ImGuiCol_TabActive] = Base03;
	style.Colors[ImGuiCol_TabUnfocused] = Base02;
	style.Colors[ImGuiCol_TabUnfocusedActive] = Base035;
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);;
	style.Colors[ImGuiCol_ScrollbarGrab] = Base02;
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = Base01;
	style.Colors[ImGuiCol_ScrollbarGrabActive] = Base015;
	style.Colors[ImGuiCol_CheckMark] = Yellow;
	style.Colors[ImGuiCol_Button] = Base02;
	style.Colors[ImGuiCol_ButtonHovered] = Base015;
	style.Colors[ImGuiCol_ButtonActive] = Base03;
	style.Colors[ImGuiCol_SliderGrab] = Yellow;
	style.Colors[ImGuiCol_SliderGrabActive] = Yellow;
	style.Colors[ImGuiCol_Header] = Base035;
	style.Colors[ImGuiCol_HeaderHovered] = Base015;
	style.Colors[ImGuiCol_HeaderActive] = Base035;
	style.Colors[ImGuiCol_Separator] = Base02;
	style.Colors[ImGuiCol_SeparatorHovered] = Base01;
	style.Colors[ImGuiCol_SeparatorActive] = Base015;
	style.Colors[ImGuiCol_ResizeGrip] = Base015;
	style.Colors[ImGuiCol_ResizeGripHovered] = Base00;
	style.Colors[ImGuiCol_ResizeGripActive] = Base01;
	style.Colors[ImGuiCol_MenuBarBg] = Base035;
	style.Colors[ImGuiCol_DockingPreview] = Base0;

	style.WindowPadding = ImVec2(8, 8);
	style.FramePadding = ImVec2(5, 5);
	style.ItemSpacing = ImVec2(12, 4);
	style.ItemInnerSpacing = ImVec2(12, 4);
	style.GrabMinSize = 6;
	style.WindowRounding = 0;	// do this to match the secondary windows
	style.ScrollbarRounding = 0;
	style.ScrollbarSize = 15;
	style.TabRounding = 0;
	style.WindowTitleAlign = ImVec2(0.5, 0.5);


	/*
	
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.93f, 0.96f, 0.95f, 0.80f);
	style.Colors[ImGuiCol_PlotLinesHovered] = grayInactive;
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.93f, 0.96f, 0.95f, 0.80f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = grayInactive;
	style.Colors[ImGuiCol_TextSelectedBg] = grayInactive;
	style.Colors[ImGuiCol_DragDropTarget] = grayInactive;
	style.Colors[ImGuiCol_NavHighlight] = grayInactive;
	style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.22f, 0.27f, 0.73f);
	*/

	return style;
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
	ImGuiIO& io = ImGui::GetIO();
	io.FontGlobalScale = app::getWindow()->getDisplay()->getContentScale() * mUIScale;

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

	// draw some GUI windows
	{
		ImGui::ScopedWindow scpWindow("Scene settings");
		if (scpWindow) {
			ImGui::DragInt2("Scene resolution", &mSceneResolution[0], 1.0, 1, 4096);
			ImGui::SliderInt("Scene MSAA", &mSceneMSAA, 0, 32);
			if (ImGui::BeginPopup("popup")) {
				ImGui::Text("This is a popup");
				ImGui::EndPopup();
			}
			if (ImGui::Button("Click me")) {
				ImGui::OpenPopup("popup");
			}
		}
	}
	{
		ImGui::ScopedWindow scpWindow("Circle parameters");
		if(scpWindow) {
			ImGui::DragFloat("Circle radius", &mCircleRadius);
		}
	}
	{
		ImGui::ScopedWindow scpWindow("UI settings");
		if (scpWindow) {
			ImGui::DragFloat("UI scale", &mUIScale, 0.01f, 0.25, 3.0);
		}
	}
	{
		ImGui::ScopedWindow scpWindow("Some other window");
		if (scpWindow) {
			static int comboIndex = 0;
			static vector<string> comboValues = { "One", "Two", "Three" };
			ImGui::Combo("Combo box", &comboIndex, comboValues, ImGuiComboFlags_None);
			ImGui::InputTextWithHint("Input text", "Type something here", &mInputString);
		}
	}

	ImGui::ShowDemoWindow();
	ImGui::ShowMetricsWindow();

	{
		ImGui::ScopedWindow scpWindow("Style");
		if (scpWindow) {
			ImGui::ShowStyleEditor();
		}
	}

	// draw our scene in a window
	// unfortunately tab ordering is not yet restored from the .ini file properly 
	// we draw the scene after the other windows to make sure its tab is selected on startup
	{
		ImGui::ScopedWindow scpWindow("Scene");
		if (scpWindow) {
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
		}
	}
}

// if msaa is disabled, all our gui windows flicker. haven't figured this one out yet
CINDER_APP( DockingUIApp, RendererGl(RendererGl::Options().msaa(2)), DockingUIApp::prepareSettings )
