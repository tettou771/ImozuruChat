#pragma once


// 文字コードの変換に必要
#include <codecvt>

// IME無効化に必要
#ifdef WIN32
#pragma comment(lib,"imm32.lib")
#endif

#include "ofMain.h"
#include "ofxJSONElement.h"
#include "AlignableFont.h"

class GoogleJapaneseInput {
public:
	GoogleJapaneseInput();
	~GoogleJapaneseInput();

	void enable();
	void disable();
	void clear();

	void keyPressed(ofKeyEventArgs &key);
	void keyReleased(ofKeyEventArgs &key);

	bool isEnabled() { return enabled; }

	string getAll();
	string getAfterHenkan();
	string getBeforeHenkan();
	void setFont(string path, float fontSize);
	void draw(ofPoint pos);
	void draw(float x, float y);

private:
	bool enabled;

	// 入力されたキーのヒストリー
	char pastPressedKey;
	u32string beforeKana; // ひらがな化する前の部分（1-2文字）
	u32string beforeHenkan; // ひらがな化したあとの部分
	u32string afterHenkan; // 変換後のテキスト

	// アルファベットの文字列をお尻だけひらがなに変換して追加するメソッド
	void alphabetToHiragana(u32string &in, u32string &out);

	ofxJSONElement json;

	// ひらがなの文字列をGoogleのAPIで変換し、選択モードに入る
	void henkan();
	// 選択された変換を確定する
	void kakutei();

	// 確定前の変換中の文字列の候補リスト
	// 描画に使うので、stringのまま持っておく
	vector<vector<u32string>> candidate;
	vector<unsigned int> candidateSelected;
	void candidateToggle(int toggle);
	void candidateFocusToggle(int toggle);
	// 変換前の文字列（APIで帰ってきた、分割後の文字列）
	vector<u32string> candidateKana;

	// 変換中のとき、フォーカスの当たっているcandidate
	int candidateFocus;

	// 変換中に、Shiftキーを押しながら左右に動かすと
	// 変換候補の切れ目が変わる。そのメソッド
	void candidateLengthChange(bool longer);
	

#ifdef WIN32
	// 互換性確保のため、UTF-32 を Shift-JIS に変換するときに使う
	// getString(), getInputText() のときに戻り値を Shift-JIS にしないと
	// 文字表示などで化けてしまうため
	string UTF32toSjis(u32string srcUTF8);
#endif

	string percentEnc(u32string str);

	// リンクエラーの出る変換器
	//wstring_convert<codecvt_utf8<char32_t>, char32_t> convert8_32;
	
	// 変換器(UTF8 UTF32)
#ifdef WIN32
	// char32_t を使うとVS2015でリンクエラーとなるので、unit32_t を使っている
	// ソース Qiita http://qiita.com/benikabocha/items/1fc76b8cea404e9591cf
	wstring_convert<codecvt_utf8<uint32_t>, uint32_t> convert8_32;
#else
    wstring_convert<codecvt_utf8<char32_t>, char32_t> convert8_32;
#endif
	string UTF32toUTF8(u32string &u32str);
	u32string UTF8toUTF32(string &str);
	// ローマ字-ひらがな変換用の辞書
	map<u32string, u32string> romajiToKana;
	void makeDictionary();

	// State
	enum State {
		Eisu,
		Kana,
		KanaNyuryoku,
		KanaHenkan
	};
	State state;

	// 描画用のフォント
	AlignableFont font;
};

