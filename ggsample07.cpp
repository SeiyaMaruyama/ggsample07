﻿// ウィンドウ関連の処理
#include "GgApplication.h"

// シェーダー関連の処理
#include "shader.h"

// 標準ライブラリ
#include <cmath>

// アニメーションの周期（秒）
const double cycle(5.0);

// 球のデータの分割数
const int slices(64);
const int stacks(32);

// 球のデータの頂点数と面数
const int vertices((slices + 1) * (stacks + 1));
const int faces(slices * stacks * 2);

//
// 球のデータの作成
//
static void makeSphere(float radius, int slices, int stacks,
  GLfloat (*pv)[3], GLfloat (*nv)[3], GLuint (*f)[3])
{
  // 頂点の位置とテクスチャ座標を求める
  for (int k = 0, j = 0; j <= stacks; ++j)
  {
    const float t(static_cast<float>(j) / static_cast<float>(stacks));
    const float ph(3.141593f * t);
    const float y(cos(ph));
    const float r(sin(ph));

    for (int i = 0; i <= slices; ++i)
    {
      const float s(static_cast<float>(i) / static_cast<float>(slices));
      const float th(-2.0f * 3.141593f * s);
      const float x(r * cos(th));
      const float z(r * sin(th));

      // 頂点の座標値
      pv[k][0] = x * radius;
      pv[k][1] = y * radius;
      pv[k][2] = z * radius;

      // 頂点の法線ベクトル
      nv[k][0] = x;
      nv[k][1] = y;
      nv[k][2] = z;

      ++k;
    }
  }

  // 面の指標を求める
  for (int k = 0, j = 0; j < stacks; ++j)
  {
    for (int i = 0; i < slices; ++i)
    {
      const int count((slices + 1) * j + i);

      // 上半分の三角形
      f[k][0] = count;
      f[k][1] = count + slices + 2;
      f[k][2] = count + 1;
      ++k;

      // 下半分の三角形
      f[k][0] = count;
      f[k][1] = count + slices + 1;
      f[k][2] = count + slices + 2;
      ++k;
    }
  }
}

//
// アプリケーションの実行
//
void GgApplication::run()
{
  // ウィンドウを作成する
  Window window("ggsample07");

  // 背景色を指定する
  glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

  // 隠面消去を有効にする
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  // プログラムオブジェクトの作成
  const GLuint program(loadProgram("ggsample07.vert", "pv", "ggsample07.frag", "fc"));

  // in (attribute) 変数のインデックスの検索（見つからなければ -1）
  const GLint nvLoc(glGetAttribLocation(program, "nv"));

  // uniform 変数のインデックスの検索（見つからなければ -1）
  const GLint mwLoc(glGetUniformLocation(program, "mw"));
  const GLint mcLoc(glGetUniformLocation(program, "mc"));
  const GLint mgLoc(glGetUniformLocation(program, "mg"));

  // ビュー変換行列を mv に求める
  const GgMatrix mv(ggLookat(0.0f, 0.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f));

  // 図形データの作成
  GLfloat pv[vertices][3];
  GLfloat nv[vertices][3];
  GLuint face[faces][3];
  makeSphere(1.0f, slices, stacks, pv, nv, face);

  // 頂点配列オブジェクトの作成
  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // 頂点バッファオブジェクトの作成
  GLuint vbo[3];
  glGenBuffers(sizeof vbo / sizeof vbo[0], vbo);

  // 頂点の座標値 pv 用のバッファオブジェクト
  glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof pv, pv, GL_STATIC_DRAW);

  // 結合されている頂点バッファオブジェクトを in 変数 pv (index == 0) から参照できるようにする
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);

  // 頂点の色 nv 用のバッファオブジェクト
  glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
  glBufferData(GL_ARRAY_BUFFER, sizeof nv, nv, GL_STATIC_DRAW);

  // 結合されている頂点バッファオブジェクトを in 変数 nv (index == nvLoc) から参照できるようにする
  glVertexAttribPointer(nvLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(nvLoc);

  // 頂点のインデックス face 用のバッファオブジェクト
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[2]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof face, face, GL_STATIC_DRAW);

  // 経過時間のリセット
  glfwSetTime(0.0);

  // ウィンドウが開いている間繰り返す
  while (window.shouldClose() == GL_FALSE)
  {
    // ウィンドウを消去する
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // シェーダプログラムの使用開始
    glUseProgram(program);

    // 時刻の計測
    const float t(static_cast<float>(fmod(glfwGetTime(), cycle) / cycle));

    // モデルビュー変換行列 (時刻 t にもとづく回転アニメーション)
    const GgMatrix mw(mv.rotateX(12.56637f * t));

    // 法線変換行列
    const GgMatrix mg(mw.normal());

    // 投影変換行列
    const GgMatrix mp(ggPerspective(0.5f, window.getAspect(), 1.0f, 15.0f));

    // モデルビュー・投影変換
    const GgMatrix mc(mp * mw);

    // uniform 変数を設定する
    glUniformMatrix4fv(mwLoc, 1, GL_FALSE, mw.get());
    glUniformMatrix4fv(mcLoc, 1, GL_FALSE, mc.get());
    glUniformMatrix4fv(mgLoc, 1, GL_FALSE, mg.get());

    // 描画に使う頂点配列オブジェクトの指定
    glBindVertexArray(vao);

    // 図形の描画
    glDrawElements(GL_TRIANGLES, faces * 3, GL_UNSIGNED_INT, 0);

    // 頂点配列オブジェクトの指定解除
    glBindVertexArray(0);

    // シェーダプログラムの使用終了
    glUseProgram(0);

    // カラーバッファを入れ替えてイベントを取り出す
    window.swapBuffers();
  }
}
