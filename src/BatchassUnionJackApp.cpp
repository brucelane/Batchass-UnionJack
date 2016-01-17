#include "BatchassUnionJackApp.h"
/*
	tempo 142
	bpm = (60 * fps) / fpb

	where bpm = beats per min
	fps = frames per second
	fpb = frames per beat

	fpb = 4, bpm = 142
	fps = 142 / 60 * 4 = 9.46
	*/
void BatchassUnionJackApp::prepare(Settings *settings)
{
	settings->setWindowSize(1440, 900);
}

void BatchassUnionJackApp::setup()
{
	mVDSettings = VDSettings::create();
	mVDSettings->mLiveCode = false;
	mVDSettings->mRenderThumbs = false;
	// utils
	mVDUtils = VDUtils::create(mVDSettings);
	// Message router
	mVDRouter = VDRouter::create(mVDSettings);

	mUseBeginEnd = false;
	updateWindowTitle();
	fpb = 16.0f;
	bpm = 142.0f;
	float fps = bpm / 60.0f * fpb;
	setFrameRate(fps);



	int w = mVDUtils->getWindowsResolution();
	setWindowSize(mVDSettings->mRenderWidth, mVDSettings->mRenderHeight);
	setWindowPos(ivec2(mVDSettings->mRenderX, mVDSettings->mRenderY));
	// UnionJack
	Color light = Color8u::hex(0x42a1eb);
	Color dark = Color8u::hex(0x082f4d);
	vec2 padding(200);
	mHorizontalAnimation = false;
	targetStr = "BATCHASS";
	strSize = targetStr.size();
	int c;
	for (size_t i = 0; i < strSize; i++)
	{
		c = Rand::randInt(48, 92);
		str.push_back(c);
		mIndexes[i] = true;
	}
	mDisplays = {
		// Let's print out the full ASCII table as a font specimen
		UnionJack(strSize).display(" !\"#$%&'()*+,-./0123456789:;<=>?").position(vec2(180, 320)).scale(8).colors(light, dark),
		UnionJack(11).display("FPS").position(padding).colors(Color8u::hex(0xf00000), Color8u::hex(0x530000))
	};
	// Position the displays relative to each other.
	mDisplays[1].below(mDisplays[0]);
	// fbo
	gl::Fbo::Format format;
	//format.setSamples( 4 ); // uncomment this to enable 4x antialiasing
	mFbo = gl::Fbo::create(FBO_WIDTH, FBO_HEIGHT, format.depthTexture());

	gl::enableDepthRead();
	gl::enableDepthWrite();
	// initialize warps
	mSettings = getAssetPath("") / "warps.xml";
	if (fs::exists(mSettings)) {
		// load warp settings from file if one exists
		mWarps = Warp::readSettings(loadFile(mSettings));
	}
	else {
		// otherwise create a warp from scratch
		mWarps.push_back(WarpPerspectiveBilinear::create());
		mWarps.push_back(WarpPerspectiveBilinear::create());
		mWarps.push_back(WarpPerspectiveBilinear::create());
	}

	mSrcArea = Area(0, 0, FBO_WIDTH, FBO_HEIGHT);
	Warp::setSize(mWarps, mFbo->getSize());
}

void BatchassUnionJackApp::cleanup()
{
	// save warp settings
	Warp::writeSettings(mWarps, writeFile(mSettings));
}

void BatchassUnionJackApp::update()
{
	mVDSettings->iFps = getAverageFps();
	mVDSettings->sFps = toString(floor(mVDSettings->iFps));
	updateWindowTitle();
	// render into our FBO
	renderSceneToFbo();
}
// Render the scene into the FBO
void BatchassUnionJackApp::renderSceneToFbo()
{
	// this will restore the old framebuffer binding when we leave this function
	// on non-OpenGL ES platforms, you can just call mFbo->unbindFramebuffer() at the end of the function
	// but this will restore the "screen" FBO on OpenGL ES, and does the right thing on both platforms
	gl::ScopedFramebuffer fbScp(mFbo);
	// clear out the FBO with blue
	gl::clear(Color(0.2, 0.0f, 0.5f));

	// setup the viewport to match the dimensions of the FBO
	gl::ScopedViewport scpVp(ivec2(0), mFbo->getSize());

	if (mHorizontalAnimation) {
		//str = "BATCHASS    BATCHASS    BATCHASS";// loops on 12
		int sz = int(getElapsedFrames() / 20.0) % 9;
		shift_left(0, sz);

	}
	else {
		for (size_t i = 0; i < strSize; i++)
		{
			if (mIndexes[i]) {
				str[i] = Rand::randInt(65, 100);
			}
			else {
				str[i] = targetStr[i];
			}
		}
	}
	mIndexes[int(getElapsedSeconds())] = false;
	mDisplays[0].display(str);
	mDisplays[1]
		.display("FPS " + mVDSettings->sFps)
		.colors(ColorA(mVDSettings->iFps < 50 ? mRed : mBlue, 0.8), ColorA(mDarkBlue, 0.8));
	mDisplays[0].draw();
	/*for (auto display = mDisplays.begin(); display != mDisplays.end(); ++display) {
		display->draw();
		}*/
	gl::color(Color::white());
}
void BatchassUnionJackApp::shift_left(std::size_t offset, std::size_t X)
{
	std::rotate(std::next(str.begin(), offset),
		std::next(str.begin(), offset + X),
		str.end());
	str = str.substr(0, str.size() - X);
}
void BatchassUnionJackApp::draw()
{
	// clear the window and set the drawing color to white
	gl::clear();
	gl::color(Color::white());

	// iterate over the warps and draw their content
	for (auto &warp : mWarps) {
		warp->draw(mFbo->getColorTexture(), mFbo->getBounds());//mSrcArea
	}


}

void BatchassUnionJackApp::resize()
{
	// tell the warps our window has been resized, so they properly scale up or down
	Warp::handleResize(mWarps);
}

void BatchassUnionJackApp::mouseMove(MouseEvent event)
{
	// pass this mouse event to the warp editor first
	if (!Warp::handleMouseMove(mWarps, event)) {
		// let your application perform its mouseMove handling here
	}
}

void BatchassUnionJackApp::mouseDown(MouseEvent event)
{
	// pass this mouse event to the warp editor first
	if (!Warp::handleMouseDown(mWarps, event)) {
		// let your application perform its mouseDown handling here
	}
}

void BatchassUnionJackApp::mouseDrag(MouseEvent event)
{
	// pass this mouse event to the warp editor first
	if (!Warp::handleMouseDrag(mWarps, event)) {
		// let your application perform its mouseDrag handling here
	}
}

void BatchassUnionJackApp::mouseUp(MouseEvent event)
{
	// pass this mouse event to the warp editor first
	if (!Warp::handleMouseUp(mWarps, event)) {
		// let your application perform its mouseUp handling here
	}
}

void BatchassUnionJackApp::keyDown(KeyEvent event)
{
	// pass this key event to the warp editor first
	if (!Warp::handleKeyDown(mWarps, event)) {
		// warp editor did not handle the key, so handle it here
		switch (event.getCode()) {
		case KeyEvent::KEY_ESCAPE:
			// quit the application
			quit();
			break;
		case KeyEvent::KEY_v:
			// toggle vertical sync
			gl::enableVerticalSync(!gl::isVerticalSyncEnabled());
			break;
		case KeyEvent::KEY_w:
			// toggle warp edit mode
			Warp::enableEditMode(!Warp::isEditModeEnabled());
			break;
		}
	}
}

void BatchassUnionJackApp::keyUp(KeyEvent event)
{
	// pass this key event to the warp editor first
	if (!Warp::handleKeyUp(mWarps, event)) {
		// let your application perform its keyUp handling here
	}
}

void BatchassUnionJackApp::updateWindowTitle()
{
	getWindow()->setTitle("(" + mVDSettings->sFps + " fps) Batchass");
}

CINDER_APP(BatchassUnionJackApp, RendererGl(RendererGl::Options().msaa(8)), &BatchassUnionJackApp::prepare)
