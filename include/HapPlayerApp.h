
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
// Utils
#include "VDUtils.h"
// Message router
#include "VDRouter.h"
// UnionJack
#include "UnionJack.h"
// spout
#include "spout.h"

using namespace ci;
using namespace ci::app;
using namespace ph::warping;
using namespace std;
using namespace VideoDromm;

template <typename T> string tostr(const T& t, int p) { ostringstream os; os << std::setprecision(p) << std::fixed << t; return os.str(); }

class HapPlayerApp : public App {
public:
	static void prepare(Settings *settings);

	void setup() override;
	void cleanup() override;
	void update() override;
	void draw() override;
	void fileDrop(FileDropEvent event) override;

	void resize() override;

	void mouseMove(MouseEvent event) override;
	void mouseDown(MouseEvent event) override;
	void mouseDrag(MouseEvent event) override;
	void mouseUp(MouseEvent event) override;

	void keyDown(KeyEvent event) override;
	void keyUp(KeyEvent event) override;

	void updateWindowTitle();
	void loadMovieFile(const fs::path &path);

	//gl::TextureRef			mInfoTexture;
	qtime::MovieGlHapRef	mMovie;
private:

	fs::path		mSettings;

	gl::TextureRef	mImage;
	WarpList		mWarps;

	Area			mSrcArea;
	// Settings
	VDSettingsRef				mVDSettings;
	// Utils
	VDUtilsRef					mVDUtils;
	// Message router
	VDRouterRef					mVDRouter;
	// UnionJack
	vector<UnionJack>			mDisplays;
	std::string					str;
	std::string					targetStr;
	int							strSize;
	void						shift_left(std::size_t offset, std::size_t X);
	Color						mBlack = Color::black();
	Color						mBlue = Color8u(66, 161, 235);
	Color						mDarkBlue = Color8u::hex(0x1A3E5A);
	Color						mRed = Color8u(240, 0, 0);
	bool						mHorizontalAnimation;
	map<int, bool>				mIndexes;
	// track 
	float						bpm;
	float						fpb;
	// fbo
	void						renderSceneToFbo();
	gl::FboRef					mFbo;
	static const int			FBO_WIDTH = 640, FBO_HEIGHT = 480;
	// lines
	void						buildMeshes();
	unsigned int				mPoints = 50;
	unsigned int				mLines = 50;
	bool						mShowHud;
	gl::BatchRef				mLineBatch;
	gl::BatchRef				mMaskBatch;

	gl::TextureRef				mTexture;
	gl::GlslProgRef				mShader;
	CameraPersp					mCamera;
	mat4						mTextureMatrix;
	// -------- SPOUT -------------
	/*SpoutSender					spoutsender;            // Create a Spout sender object
	bool						bInitialized;           // true if a sender initializes OK
	bool						bMemoryMode;            // tells us if texture share compatible
	unsigned int				g_Width, g_Height;      // size of the texture being sent out
	char						SenderName[256];        // sender name 
	gl::TextureRef				spoutTexture;           // Local Cinder texture used for sharing
	bool						bDoneOnce;				// only try to initialize once
	int							nSenders;*/
	// ----------------------------
	bool						mLoopVideo;
};
