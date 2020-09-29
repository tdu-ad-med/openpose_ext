#include <OpenPoseWrapper/MinimumOpenPose.h>
#include <OpenPoseWrapper/Examples/SqlOpenPoseEvent.h>
#include <OpenPoseWrapper/Examples/VideoOpenPoseEvent.h>
#include <OpenPoseWrapper/Examples/PlotInfoOpenPoseEvent.h>
#include <OpenPoseWrapper/Examples/TrackingOpenPoseEvent.h>
#include <OpenPoseWrapper/Examples/PeopleCounterOpenPoseEvent.h>
#include <OpenPoseWrapper/Examples/PreviewOpenPoseEvent.h>
#include <regex>

class CustomOpenPoseEvent : public OpenPoseEvent
{
private:
	std::shared_ptr<VideoOpenPoseEvent> video;
	std::shared_ptr<TrackingOpenPoseEvent> tracker;
	std::shared_ptr<SqlOpenPoseEvent> sql;
	std::shared_ptr<PreviewOpenPoseEvent> preview;
	vt::ScreenToGround screenToGround;
	cv::Point mouse;
	int previewMode;

	int checkError()
	{
		if ((!video) || (!tracker))
		{
			std::cout
				<< "VideoOpenPoseEvent, TrackingOpenPoseEventのいずれかが未指定です。\n"
				<< "setParams関数で正しい値を指定してください。"
				<< std::endl;
			return 1;
		}
		return 0;
	}
public:
	CustomOpenPoseEvent() : previewMode(0) {}
	virtual ~CustomOpenPoseEvent() {}
	int init() override final
	{
		if (checkError()) return 1;

		// people_with_normalized_trackingテーブルを再生成
		if (sql->deleteTableIfExist(u8"people_with_normalized_tracking")) return 1;
		if (sql->createTableIfNoExist(u8"people_with_normalized_tracking", u8"frame INTEGER, people INTEGER, x REAL, y REAL")) return 1;
		if (sql->createIndexIfNoExist(u8"people_with_normalized_tracking", u8"frame", false)) return 1;
		if (sql->createIndexIfNoExist(u8"people_with_normalized_tracking", u8"people", false)) return 1;
		if (sql->createIndexIfNoExist(u8"people_with_normalized_tracking", u8"frame", u8"people", true)) return 1;

		// カメラキャリブレーションの設定
		screenToGround.setCalibration(
			1858.0, 1044.0, 0.5,  // カメラキャリブレーションに用いた画像の解像度(w, h), 出力画像の拡大率
			1057, 1057, 935, 567,  // カメラ内部パラメータの焦点距離と中心座標(fx, fy, cx, cy)
			0.0, 0.0, 0.0, 0.0  // カメラの歪み係数(k1, k2, k3, k4)
		);

		return 0;
	}
	int sendImageInfo(ImageInfo& imageInfo, std::function<void(void)> exit) override final { return 0; }
	int recieveImageInfo(ImageInfo& imageInfo, std::function<void(void)> exit) override final
	{
		if (checkError()) return 1;

		// カメラの垂直画角
		double cam_h_fov = 112;

		// カメラキャリブレーションの設定
		screenToGround.setCalibration(
			1858.0, 1044.0, 0.5,  // カメラキャリブレーションに用いた画像の解像度(w, h), 出力画像の拡大率
			1057, 1057, mouse.x, 567,  // カメラ内部パラメータの焦点距離と中心座標(fx, fy, cx, cy)
			0.0, 0.0, 0.0, 0.0  // カメラの歪み係数(k1, k2, k3, k4)
		);

		// 複製
		imageInfo.outputImage = imageInfo.outputImage.clone();

		// 射影変換
		screenToGround.setParams(
			imageInfo.outputImage.cols, imageInfo.outputImage.rows, cam_h_fov, 6.3,
			346, 659,
			1056, 668,
			990, 202,
			478, 292
		);
		
		if (previewMode == 1) imageInfo.outputImage = screenToGround.onlyFlatMat(imageInfo.outputImage);  // 変形
		if (previewMode == 2) imageInfo.outputImage = screenToGround.translateMat(imageInfo.outputImage, 0.3f);  // 変形

		// people_with_normalized_trackingテーブルの更新
		if (!sql->isDataExist("people_with_normalized_tracking", "frame", imageInfo.frameNumber))
		{
			SQLite::Statement insertQuery(*sql->database, u8"INSERT INTO people_with_normalized_tracking VALUES (?, ?, ?, ?)");
			for (auto&& currentPerson = tracker->latestPeople.begin(); currentPerson != tracker->latestPeople.end(); currentPerson++)
			{
				auto&& position = TrackingOpenPoseEvent::getJointAverage(currentPerson->second);
				auto normal = screenToGround.translate(vt::Vector4{ position.x, position.y });
				if (sql->bindAllAndExec(insertQuery, imageInfo.frameNumber, currentPerson->first, normal.x, normal.y)) return 1;
			}
		}

		return 0;
	}
	void setParams(
		const std::shared_ptr<VideoOpenPoseEvent> videoTmp,
		const std::shared_ptr<TrackingOpenPoseEvent> trackingTmp,
		const std::shared_ptr<PreviewOpenPoseEvent> previewTmp = nullptr
	)
	{
		video = videoTmp;
		tracker = trackingTmp;
		sql = trackingTmp->sql;
		preview = previewTmp;

		if (checkError()) return;

		if (!preview) return;

		// マウスイベント処理
		preview->addMouseEventListener([&](int event, int x, int y) {
			switch (event)
			{
				// マウス移動時
			case cv::EVENT_MOUSEMOVE:
				mouse.x = x; mouse.y = y;
				break;

				// 左クリック時
			case cv::EVENT_LBUTTONDOWN:
				std::cout << x << ", " << y << std::endl;
				break;

				// 右クリック時
			case cv::EVENT_RBUTTONDOWN:
				break;

				// その他
			default:
				break;
			}
			});

		// キーイベント処理
		preview->addKeyboardEventListener([&](int key) {
			switch (key)
			{
				// Jキーで30フレーム戻る
			case 'j':
				video->seekRelative(-30);
				break;

				// Kキーで30フレーム進む
			case 'k':
				video->seekRelative(30);
				break;

				// Aキーで画面表示切替
			case 'a':
				previewMode = (previewMode + 1) % 3;
				break;

				// スペースキーで動画の再生/一時停止
			case 32:
				if (video->isPlay()) video->pause();
				else video->play();
				break;

				// その他
			default:
				break;
			}
		});
	}
};

int main(int argc, char* argv[])
{
	// openposeのラッパークラス
	MinimumOpenPose mop;



	// 入力する映像ファイルのフルパス
	std::string videoPath = R"(media/video.mp4)";
	if (argc == 2) videoPath = argv[1];

	// 入出力するsqlファイルのフルパス
	std::string sqlPath = videoPath + ".sqlite3";



	// SQL入出力機能の追加
	auto sql = mop.addEventListener<SqlOpenPoseEvent>(sqlPath, 300);

	// 動画読み込み処理の追加
	auto video = mop.addEventListener<VideoOpenPoseEvent>(videoPath);

	// 骨格のトラッキング処理の追加
	auto tracker = mop.addEventListener<TrackingOpenPoseEvent>(sql);

	// 人数カウント処理の追加
	(void)mop.addEventListener<PeopleCounterOpenPoseEvent>(tracker, 579, 578, 1429, 577, 100.0, true);

	// 出力画像に骨格情報などを描画する処理の追加
	(void)mop.addEventListener<PlotInfoOpenPoseEvent>(true, true, false);

	// 自分で定義したイベントリスナーの登録
	auto custom = mop.addEventListener<CustomOpenPoseEvent>();

	// 出力画像のプレビューウィンドウを生成する処理の追加
	std::shared_ptr<PreviewOpenPoseEvent> preview;
	preview = mop.addEventListener<PreviewOpenPoseEvent>("result");



	custom->setParams(video, tracker, preview);



	// openposeの起動
	auto start = std::chrono::high_resolution_clock::now();
	int ret = mop.startup();
	double time = (1.0 / 1000.0) * (double)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();



	// 起動から終了までに要した時間の表示
	std::cout << "time score : " << time << " sec." << std::endl;



	return ret;
}
