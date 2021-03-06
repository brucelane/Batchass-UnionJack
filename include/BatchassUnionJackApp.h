#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/gl/Texture.h"
#include "cinder/ImageIo.h"
#include "cinder/Rand.h"

#include "cinder/gl/Fbo.h"
#include "Resources.h"
#include "MovieHap.h"

// warping
#include "Warp.h"
// parameters
#include "VDSettings.h"
// Session
#include "VDSession.h"
// Utils
/*#include "VDUtils.h"
// Animation
#include "VDAnimation.h"
// Textures
#include "VDTexture.h"
// Message router
#include "VDRouter.h"*/
// UnionJack
#include "UnionJack.h"
// spout
#include "spout.h"

using namespace ci;
using namespace ci::app;
using namespace ph::warping;
using namespace std;
using namespace VideoDromm;

class BatchassUnionJackApp : public App {

public:
	static void prepare(Settings *settings);

	void setup() override;

	void update() override;
	void draw() override;
	void cleanup() override;

	void resize() override;

	void mouseMove(MouseEvent event) override;
	void mouseDown(MouseEvent event) override;
	void mouseDrag(MouseEvent event) override;
	void mouseUp(MouseEvent event) override;

	void keyDown(KeyEvent event) override;
	void keyUp(KeyEvent event) override;

	void						fileDrop(FileDropEvent event) override;
	void						loadMovieFile(const fs::path &path);

	void updateWindowTitle();
private:
	fs::path		mSettings;

	WarpList		mWarps;

	Area			mSrcArea;
	// Settings
	VDSettingsRef				mVDSettings;
	// Session
	VDSessionRef				mVDSession;
	// Utils
	/*VDUtilsRef					mVDUtils;
	// Animation
	VDAnimationRef				mVDAnimation;
	// Message router
	VDRouterRef					mVDRouter;
	// Textures
	VDTextureList				mTexs;
	fs::path					mTexturesFilepath;*/

	// UnionJack
	vector<UnionJack>			mDisplays;
	std::string					str;
	std::string					targetStr;
	int							strSize;
	void						shift_left(std::size_t offset, std::size_t X);
	Color						mBlack = Color::black();
	Color						mBlue = Color8u(99, 0, 235);
	Color						mDarkBlue = Color8u::hex(0x1A3E5A);
	Color						mRed = Color8u(240, 0, 0);
	bool						mHorizontalAnimation;
	map<int, bool>				mIndexes;
	gl::TextureRef				mTexture;
	// track 
	float						bpm;
	float						fpb;
	// fbo
	void						renderSceneToFbo();
	gl::FboRef					mFbo;
	static const int			FBO_WIDTH = 1024, FBO_HEIGHT = 768;
	// lines
	void						buildMeshes();
	unsigned int				mPoints = 50;
	unsigned int				mLines = 50;
	bool						mShowHud;
	gl::BatchRef				mLineBatch;
	gl::BatchRef				mMaskBatch;

	gl::GlslProgRef				mShader;
	CameraPersp					mCamera;
	mat4						mTextureMatrix;
	// hap
	qtime::MovieGlHapRef		mMovie;
	bool						mLoopVideo;
	// -------- SPOUT -------------
	SpoutSender					spoutsender;            // Create a Spout sender object
	bool						bInitialized;           // true if a sender initializes OK
	bool						bMemoryMode;            // tells us if texture share compatible
	unsigned int				g_Width, g_Height;      // size of the texture being sent out
	char						SenderName[256];        // sender name
	gl::TextureRef				spoutTexture;           // Local Cinder texture used for sharing
	bool						bDoneOnce;				// only try to initialize once
	int							nSenders;
	// ----------------------------
	// mouse
	float						mouseX, mouseY;
};
