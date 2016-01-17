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
void BatchassUnionJackApp::prepare( Settings *settings )
{
	settings->setWindowSize( 1440, 900 );
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
	fpb = 4.0f;
	bpm = 142.0f;
	float fps = bpm / 60.0f * fpb;
	setFrameRate(fps);

	// initialize warps
	mSettings = getAssetPath( "" ) / "warps.xml";
	if( fs::exists( mSettings ) ) {
		// load warp settings from file if one exists
		mWarps = Warp::readSettings( loadFile( mSettings ) );
	}
	else {
		// otherwise create a warp from scratch
		mWarps.push_back(WarpPerspectiveBilinear::create());
		mWarps.push_back(WarpPerspectiveBilinear::create());
		mWarps.push_back(WarpPerspectiveBilinear::create());
	}

	// load test image
	try {
		mImage = gl::Texture::create( loadImage( loadAsset( "help.jpg" ) ), 
									  gl::Texture2d::Format().loadTopDown().mipmap( true ).minFilter( GL_LINEAR_MIPMAP_LINEAR ) );

		mSrcArea = mImage->getBounds();

		// adjust the content size of the warps
		Warp::setSize( mWarps, mImage->getSize() );
	}
	catch( const std::exception &e ) {
		console() << e.what() << std::endl;
	}
	int w = mVDUtils->getWindowsResolution();
	setWindowSize(mVDSettings->mRenderWidth, mVDSettings->mRenderHeight);
	setWindowPos(ivec2(mVDSettings->mRenderX, mVDSettings->mRenderY));
}

void BatchassUnionJackApp::cleanup()
{
	// save warp settings
	Warp::writeSettings( mWarps, writeFile( mSettings ) );
}

void BatchassUnionJackApp::update()
{
	// there is nothing to update
}

void BatchassUnionJackApp::draw()
{
	// clear the window and set the drawing color to white
	gl::clear();
	gl::color( Color::white() );

	if( mImage ) {
		// iterate over the warps and draw their content
		for( auto &warp : mWarps ) {
			// there are two ways you can use the warps:
			if( mUseBeginEnd ) {
				// a) issue your draw commands between begin() and end() statements
				warp->begin();

				// in this demo, we want to draw a specific area of our image,
				// but if you want to draw the whole image, you can simply use: gl::draw( mImage );
				gl::draw( mImage, mSrcArea, warp->getBounds() );

				warp->end();
			}
			else {
				// b) simply draw a texture on them (ideal for video)

				// in this demo, we want to draw a specific area of our image,
				// but if you want to draw the whole image, you can simply use: warp->draw( mImage );
				warp->draw( mImage, mSrcArea );
			}
		}
	}
}

void BatchassUnionJackApp::resize()
{
	// tell the warps our window has been resized, so they properly scale up or down
	Warp::handleResize( mWarps );
}

void BatchassUnionJackApp::mouseMove( MouseEvent event )
{
	// pass this mouse event to the warp editor first
	if( !Warp::handleMouseMove( mWarps, event ) ) {
		// let your application perform its mouseMove handling here
	}
}

void BatchassUnionJackApp::mouseDown( MouseEvent event )
{
	// pass this mouse event to the warp editor first
	if( !Warp::handleMouseDown( mWarps, event ) ) {
		// let your application perform its mouseDown handling here
	}
}

void BatchassUnionJackApp::mouseDrag( MouseEvent event )
{
	// pass this mouse event to the warp editor first
	if( !Warp::handleMouseDrag( mWarps, event ) ) {
		// let your application perform its mouseDrag handling here
	}
}

void BatchassUnionJackApp::mouseUp( MouseEvent event )
{
	// pass this mouse event to the warp editor first
	if( !Warp::handleMouseUp( mWarps, event ) ) {
		// let your application perform its mouseUp handling here
	}
}

void BatchassUnionJackApp::keyDown( KeyEvent event )
{
	// pass this key event to the warp editor first
	if( !Warp::handleKeyDown( mWarps, event ) ) {
		// warp editor did not handle the key, so handle it here
		switch( event.getCode() ) {
			case KeyEvent::KEY_ESCAPE:
				// quit the application
				quit();
				break;
			case KeyEvent::KEY_f:
				// toggle full screen
				setFullScreen( !isFullScreen() );
				break;
			case KeyEvent::KEY_v:
				// toggle vertical sync
				gl::enableVerticalSync( !gl::isVerticalSyncEnabled() );
				break;
			case KeyEvent::KEY_w:
				// toggle warp edit mode
				Warp::enableEditMode( !Warp::isEditModeEnabled() );
				break;
			case KeyEvent::KEY_a:
				// toggle drawing a random region of the image
				if( mSrcArea.getWidth() != mImage->getWidth() || mSrcArea.getHeight() != mImage->getHeight() )
					mSrcArea = mImage->getBounds();
				else {
					int x1 = Rand::randInt( 0, mImage->getWidth() - 150 );
					int y1 = Rand::randInt( 0, mImage->getHeight() - 150 );
					int x2 = Rand::randInt( x1 + 150, mImage->getWidth() );
					int y2 = Rand::randInt( y1 + 150, mImage->getHeight() );
					mSrcArea = Area( x1, y1, x2, y2 );
				}
				break;
			case KeyEvent::KEY_SPACE:
				// toggle drawing mode
				mUseBeginEnd = !mUseBeginEnd;
				updateWindowTitle();
				break;
		}
	}
}

void BatchassUnionJackApp::keyUp( KeyEvent event )
{
	// pass this key event to the warp editor first
	if( !Warp::handleKeyUp( mWarps, event ) ) {
		// let your application perform its keyUp handling here
	}
}

void BatchassUnionJackApp::updateWindowTitle()
{
	if( mUseBeginEnd )
		getWindow()->setTitle( "Warping Sample - Using begin() and end()" );
	else
		getWindow()->setTitle( "Warping Sample - Using draw()" );
}

CINDER_APP( BatchassUnionJackApp, RendererGl( RendererGl::Options().msaa( 8 ) ), &BatchassUnionJackApp::prepare )
