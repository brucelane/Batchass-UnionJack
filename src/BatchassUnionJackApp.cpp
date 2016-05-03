
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
	// Session
	mVDSession = VDSession::create(mVDSettings);
	// Animation
	mVDAnimation = VDAnimation::create(mVDSettings, mVDSession);
	// Message router
	//mVDRouter = VDRouter::create(mVDSettings, mVDAnimation, mVDSession);


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
		UnionJack(strSize).display(" !\"#$%&'()*+,-./0123456789:;<=>?").position(vec2(50, 280)).scale(7).colors(light, dark),
		UnionJack(strSize).display("FPS").position(padding).scale(8).colors(Color8u::hex(0xf00000), Color8u::hex(0x530000))
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
	// lines
	try {
		mTexture = gl::Texture::create(loadImage(loadAsset("mandala1.png")));
		mTexture->bind(0);
	}
	catch (...) {
		console() << "unable to load the texture file!" << std::endl;
	}
	mTexture->setWrap(GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);

	try {
		mShader = ci::gl::GlslProg::create(
			ci::app::loadAsset("vert.glsl"),
			ci::app::loadAsset("frag.glsl")
			);
	}
	catch (gl::GlslProgCompileExc &exc) {
		console() << "Shader compile error: " << std::endl;
		console() << exc.what();
	}
	catch (...) {
		console() << "Unable to load shader" << std::endl;
	}
	mShowHud = true;
	g_Width = FBO_WIDTH;
	g_Height = FBO_HEIGHT;
	if (!bInitialized) {
		strcpy_s(SenderName, "Batchass UnionJack Spout Sender"); // we have to set a sender name first
	}
	spoutTexture = gl::Texture::create(g_Width, g_Height);
	// load image
	try {
		mImage = gl::Texture::create(loadImage(loadAsset("mandala1.png")),
			gl::Texture2d::Format().loadTopDown().mipmap(true).minFilter(GL_LINEAR_MIPMAP_LINEAR));

		mSrcArea = mImage->getBounds();

		// adjust the content size of the warps
		Warp::setSize(mWarps, mImage->getSize());
	}
	catch (const std::exception &e) {
		console() << e.what() << std::endl;
	}
	buildMeshes();
}
void BatchassUnionJackApp::buildMeshes()
{
	vector<vec3> lineCoords;
	vector<vec3> maskCoords;

	for (unsigned int z = 0; z < mLines; ++z) {
		for (unsigned int x = 0; x < mPoints; ++x) {
			vec3 vert = vec3(x / (float)mPoints, 1, z / (float)mLines);

			lineCoords.push_back(vert);

			// To speed up the vertex shader it only does the texture lookup
			// for vertexes with y values greater than 0. This way we can build
			// a strip: 1 1 1  that will become: 2 9 3
			//          |\|\|                    |\|\|
			//          0 0 0                    0 0 0
			maskCoords.push_back(vert);
			vert.y = 0.0;
			maskCoords.push_back(vert);
		}
	}
	gl::VboMeshRef lineMesh = gl::VboMesh::create(lineCoords.size(), GL_LINE_STRIP, {
		gl::VboMesh::Layout().usage(GL_STATIC_DRAW).attrib(geom::Attrib::POSITION, 3),
	});
	lineMesh->bufferAttrib(geom::Attrib::POSITION, lineCoords);
	mLineBatch = gl::Batch::create(lineMesh, mShader);

	gl::VboMeshRef maskMesh = gl::VboMesh::create(maskCoords.size(), GL_TRIANGLE_STRIP, {
		gl::VboMesh::Layout().usage(GL_STATIC_DRAW).attrib(geom::Attrib::POSITION, 3),
	});
	maskMesh->bufferAttrib(geom::Attrib::POSITION, maskCoords);
	mMaskBatch = gl::Batch::create(maskMesh, mShader);
}
void BatchassUnionJackApp::update()
{
	mVDSettings->iFps = getAverageFps();
	mVDSettings->sFps = toString(floor(mVDSettings->iFps));
	//mVDRouter->update();
	updateWindowTitle();
	//float scale = math<float>::clamp(mShip.mPos.z, 0.2, 1.0);
	float scale = 1.0f;
	mTextureMatrix = glm::translate(vec3(0.5, 0.5, 0));
	mTextureMatrix = glm::rotate(mTextureMatrix, mVDSettings->liveMeter, vec3(0, 0, 1));
	mTextureMatrix = glm::scale(mTextureMatrix, vec3(scale, scale, 0.25));
	//mTextureMatrix = glm::translate(mTextureMatrix, vec3(mVDSettings->liveMeter, mVDSettings->iBeat, 0));
	mTextureMatrix = glm::translate(mTextureMatrix, vec3(-0.5, -0.5, 0));

	mCamera.setPerspective(40.0f, 1.0f, 0.5f, 3.0f);
	//mCamera.lookAt(vec3(0.0f, 1.5f, 1.0f), vec3(0.0, 0.1, 0.0), vec3(0, 1, 0));
	mCamera.lookAt(vec3(0.0f, 2.0f, 1.0f), vec3(0.0, 0.1, 0.0), vec3(0, 1, 0));
	//if (mVDSettings->iBeat == 303) loadMovieFile(getAssetPath("") / "pupilles1024.hap.mov");
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
	// clear out the FBO with white or black
	switch (mVDSettings->iBeat)
	{
	case 80:
	case 95:
	case 96:
	case 112:
	case 127:
	case 128:
	case 144:
	case 159:
	case 160:
	case 176:
		gl::clear(Color(1.0, 1.0f, 1.0f), true);
		break;

	default:
		gl::clear(mBlack, true);
		break;
	}

	// setup the viewport to match the dimensions of the FBO
	gl::ScopedViewport scpVp(ivec2(0), mFbo->getSize());

	if (mShowHud) {
		if (mHorizontalAnimation) {
			str = "BATCHASS    BATCHASS    BATCHASS";// loops on 12
			int sz = int(getElapsedFrames() / 20.0) % 13;
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

		if (mVDSettings->iBeat > 0 && mVDSettings->iBeat / 8 < strSize) mIndexes[mVDSettings->iBeat / 8] = false;
		if (mVDSettings->iBeat > 63) mHorizontalAnimation = true;
		if (mVDSettings->iBeat > 176) mShowHud = false;
		mDisplays[0].display(str);
		mDisplays[1]
			.display("Beat " + toString(mVDSettings->iBeat))
			.colors(ColorA(mVDSettings->iFps < 50 ? mRed : mBlue, 0.8), ColorA(mDarkBlue, 0.8));
		mDisplays[0].draw();
		/*for (auto display = mDisplays.begin(); display != mDisplays.end(); ++display) {
		display->draw();
		}*/
		gl::color(Color::white());
	}
	else {

		gl::ScopedMatrices matrixScope;
		gl::setMatrices(mCamera);

		gl::ScopedDepth depthScope(true);

		mShader->uniform("textureMatrix", mTextureMatrix);

		// Center the model
		gl::translate(-0.5, 0.0, -0.5);

		unsigned int indiciesInLine = mPoints;
		unsigned int indiciesInMask = mPoints * 2;
		// Draw front to back to take advantage of the depth buffer.
		for (int i = mLines - 1; i >= 0; --i) {
			gl::color(mBlack);
			// Draw masks with alternating colors for debugging
			// gl::color( Color::gray( i % 2 == 1 ? 0.5 : 0.25) );
			mMaskBatch->draw(i * indiciesInMask, indiciesInMask);

			gl::color(mBlue);
			mLineBatch->draw(i * indiciesInLine, indiciesInLine);
		}

	}

}
void BatchassUnionJackApp::shift_left(std::size_t offset, std::size_t X)
{
	std::rotate(std::next(str.begin(), offset),
		std::next(str.begin(), offset + X),
		str.end());
	str = str.substr(0, str.size() - X);
}
void BatchassUnionJackApp::cleanup()
{
	// save warp settings
	Warp::writeSettings(mWarps, writeFile(mSettings));
	spoutsender.ReleaseSender();
	quit();
}

void BatchassUnionJackApp::draw()
{
	gl::clear(Color::black());
	gl::color(Color::white());
	int i = 0;
	// iterate over the warps and draw their content
	for (auto &warp : mWarps) {
		if (i == 0) {
			warp->draw(mFbo->getColorTexture(), mFbo->getBounds());
		}
		else {
			warp->draw(mImage, mSrcArea);
		}
		i++;
	}
	if (bInitialized)
	{
		spoutsender.SendTexture(mFbo->getColorTexture()->getId(), mFbo->getColorTexture()->getTarget(), g_Width, g_Height);
		// Grab the screen (current read buffer) into the local spout texture
		//spoutTexture->bind();
		//glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, g_Width, g_Height);
		//spoutTexture->unbind();

		// Send the texture for all receivers to use
		// NOTE : if SendTexture is called with a framebuffer object bound, that binding will be lost
		// and has to be restored afterwards because Spout uses an fbo for intermediate rendering
		//spoutsender.SendTexture(spoutTexture->getId(), spoutTexture->getTarget(), g_Width, g_Height);
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
	getWindow()->setTitle("(" + mVDSettings->sFps + " fps) " + toString(mVDSettings->iBeat) + " HapPlayer");

}


CINDER_APP(BatchassUnionJackApp, RendererGl(RendererGl::Options().msaa(8)), &BatchassUnionJackApp::prepare)
