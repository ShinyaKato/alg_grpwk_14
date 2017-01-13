#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>


/* 定数 */
#define N 500001


int main_prg(int, char**);
int sortS(const void*, const void*);
void makeBMTable1(char*, int, int*);
void makeBMTable2(char*, int, int*);
int formalBMStrstr(char*, int, char*, int, int);
char other(char, char);
char another(char);
int fill(char*);


/* 文字列の先頭へのポインタと長さを格納するための構造体 */
struct data {
  char* string;
  int length;
};
typedef struct data Data;


char sourceT[N], sourceS[N], sourceR[N]; /* 入力と出力を格納する文字列 */
Data T, S[N], R; /* 先頭ポインタと文字列長 */
int D; /* Sの断片数 */


/* main function */
int main(int argc, char** argv){
  struct rusage u;
  struct timeval start, end;

  getrusage(RUSAGE_SELF, &u);
  start = u.ru_utime;

  main_prg(argc, argv);

  getrusage(RUSAGE_SELF, &u );
  end = u.ru_utime;

  fprintf(stderr, "%lf\n", (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)*1.0E-6);

  return(0);
}

/* implement here */
int main_prg(int argc, char** argv){
  int i, j, idx;

  /* 1. 標準入力からデータを読み込む */
  scanf("%s", sourceT); /* Tを読み込む */
  T.string = sourceT;
  T.length = N;
  for(i = 0, D = 0; scanf("%s", &sourceS[i]) != EOF; D++) { /* S[i]を読み込む */
    S[D].string = &sourceS[i];
    S[D].length = strlen(&sourceS[i]);
    i += S[D].length;
  }
  R.string = sourceR;
  R.length = N;
  for(i = 0; i < N; i++) R.string[i] = T.string[i]; /* TをRにコピー */

  /* 2. 前処理(Sを断片の長さが長い順にソート) */
  qsort((void*) S, D, sizeof(Data), sortS);

  /* 3. BM法を用いてS[i]がTにマッチする箇所を列挙, もっともマッチする場所にコピー */
  for(i = 0; i < D && S[i].length >= 30; i++) { /* 長さ17以上のS[i]だけつかう */
    idx = formalBMStrstr(T.string, T.length, S[i].string, S[i].length, 0); /* もっともマッチする場所を求める */
    if(idx == -1) continue; /* マッチする場所が見つからなかった場合にはスキップ */
    for(j = 0; j < S[i].length; j++) {
      R.string[idx + j] = S[i].string[j]; /* マッチした場所にS[i]をコピー */
      T.string[idx + j] = 'z'; /* Tの方のマッチした部分にはダミーのzを入れておく(一度マッチした場所にはマッチしなくなる, BM法で勝手にスキップされる) */
    }
  }

  /* 4. 確率的にもっとも精度が高くなるように残った'x'を埋めていく */
  /* fill(R.string); */

  /* 5. 結果を出力 */
  printf("%s\n", R.string);

  return(0);
}

/* Sの断片をその長さ順にソートするための関数 */
int sortS(const void* a, const void* b) {
  return ((Data*) b)->length - ((Data*) a)->length;
}

/**
 * BM法の移動量を求める関数(Bad Character Ruleに基づく移動量)
 * char* pattern: 探索するパターン文字列
 * int length: patternの長さ
 * int* bmTable: 移動量を格納するための配列(長さは256)
 **/
void makeBMTable1(char* pattern, int length, int* bmTable) {
  char* patIt;
  int counter;

  for(counter = 0; counter < 256; counter++)
    bmTable[counter] = 0;

  for(patIt = pattern, counter = length; counter > 1;)
    bmTable[(int) *patIt++] = --counter;
}

/**
 * BM法の移動量を求める関数(Good Suffix Ruleに基づく移動量)
 * char* pattern: 探索するパターン文字列
 * int length: patternの長さ
 * int* bmTable: 移動量を格納するための配列(長さはpatternの文字数)
 **/
void makeBMTable2(char* pattern, int patLen, int* bmTable) {
  int sIdx1, sIdx0, nextIdx[100000], nextIdxSize, sLen, curIdx1, i, j;

  for(i = 0; i < patLen; i++) bmTable[i] = -1;
  sIdx1 = patLen;
  if(sIdx1 == 0) return;

  for(nextIdxSize = 0, sIdx0 = sIdx1 - 1; sIdx0 > 0 ; --sIdx0) {
    if(pattern[sIdx0 - 1] == pattern[sIdx1 - 1]) {
      nextIdx[nextIdxSize++] = sIdx0 - 1;
    } else if(bmTable[sIdx1 - 1] == -1) {
      bmTable[sIdx1 - 1] = patLen - sIdx0;
    }
  }
  if(bmTable[sIdx1 - 1] == -1) bmTable[sIdx1 - 1] = patLen;

  sLen = 2;
  for(--sIdx1; sIdx1 > 0; --sIdx1) {
    for(i = 0; i < nextIdxSize;) {
      sIdx0 = nextIdx[i];
      if(sIdx0 == 0 || pattern[sIdx0 - 1] != pattern[sIdx1 - 1]) {
        for(j = i + 1; j < nextIdxSize; j++) nextIdx[j - 1] = nextIdx[j];
        --nextIdxSize;
        if(bmTable[sIdx1 - 1] == -1) bmTable[sIdx1 - 1] = patLen - (sIdx0 + sLen - 1);
      } else {
        --nextIdx[i];
        ++i;
      }
    }
    if(nextIdxSize == 0) break;
    ++sLen;
  }

  sIdx1 = patLen;
  curIdx1 = -1;
  while (sIdx1 > 1) {
    --sIdx1;
    if(bmTable[sIdx1 - 1] != -1) continue;
    sLen = patLen - sIdx1;
    i = 0;
    while(sLen > 0) {
      if(strncmp(pattern, pattern + sIdx1 + i, sLen) == 0) break;
      ++i;
      --sLen;
    }
    if(curIdx1 == patLen - sLen) break;
    bmTable[sIdx1 - 1] = curIdx1 = patLen - sLen;
  }

  while(sIdx1 > 0) {
    if(bmTable[sIdx1 - 1] == -1) bmTable[sIdx1 - 1] = curIdx1;
    --sIdx1;
  }
}

/**
 * BM法による文字列マッチング関数(Bad Character RuleとGood Suffix Ruleに基づく)
 * char* text: 探索される文字列
 * int txtLen: textの長さ
 * char* pattern: 探索するパターン文字列
 * int patLen: patternの長さ
 * int txtIdx: 探索を開始する位置
 * return: もっともマッチした位置
 **/
int formalBMStrstr(char* text, int txtLen, char* pattern, int patLen, int txtIdx) {
  int bmTable1[256], bmTable2[1000], patIdx, shtIdx, shtIdx1, shtIdx2, i1, i2, includeX, maxMatch, match, maxIdx;

  if(txtLen < patLen) return -1;
  txtLen -= patLen;

  makeBMTable1(pattern, patLen, bmTable1);
  makeBMTable2(pattern, patLen, bmTable2);

  maxMatch = 0;
  maxIdx = -1;
  while(txtIdx <= txtLen) {
    match = 0;
    includeX = 0;
    patIdx = patLen;
    for(; patIdx > 0; --patIdx) {
      if(text[txtIdx + patIdx - 1] != pattern[patIdx - 1] && text[txtIdx + patIdx - 1] != 'x') break;
      if(text[txtIdx + patIdx - 1] == 'x') includeX = 1;
      else ++match;
    }

    if(patIdx == 0) {
      if(maxMatch < match) {
        maxMatch = match;
        maxIdx = txtIdx;
      }
      ++txtIdx;
      continue;
    }

    i1 = bmTable1[(int) text[txtIdx + patIdx - 1]];
    shtIdx1 = patIdx;
    if(i1 > 0) {
      i2 = patLen - patIdx;
      shtIdx1 = (i1 < i2) ? 1 : i1 - i2;
    }
    shtIdx2 = (includeX == 0) ? bmTable2[patIdx - 1] : 1;
    shtIdx = (shtIdx1 > shtIdx2) ? shtIdx1 : shtIdx2;
    txtIdx += shtIdx;
  }

  return maxIdx;
}

/* 'a', 'b', 'c'の中からxでもyでもないものを返す関数 */
char other(char x, char y) {
  if(x == 'a' && y == 'b') return 'c';
  if(x == 'a' && y == 'c') return 'b';
  if(x == 'b' && y == 'c') return 'a';
  if(x == 'b' && y == 'a') return 'c';
  if(x == 'c' && y == 'a') return 'b';
  return 'a';
}

/* 'a', 'b', 'c'の中からx以外の文字を返す関数 */
char another(char x) {
  if(x == 'a') return 'b';
  if(x == 'b') return 'c';
  return 'a';
}

/* 確率的にもっとも当てはまる可能性が高い文字で'x'を置換する */
int fill(char* T) {
  int cnt, i, j, k;

  for(i = 0; i < N; i++)
    if(T[i] != 'x') break;

  for(; i < N; i++) {
    cnt = 1;
    if(T[i] == 'x') {
      for(j = i; j < N; j++) {
        if(T[j] != 'x') {
          if(T[i - 1] != T[j]) {
            cnt = j - i;
            if(cnt == 1) { T[i] = other(T[i - 1], T[j]);
            } else if(cnt == 2) { T[i] = T[j]; T[i + 1] = T[i - 1];
            } else if(cnt == 3) { T[i] = other(T[i - 1], T[j]); T[i + 1] = T[j]; T[i + 2] = other(T[i - 1], T[j]);
            } else if(cnt == 5) { T[i] = other(T[i - 1], T[j]); T[i + 1] = T[i - 1]; T[i + 2] = T[i]; T[i + 3] = T[j]; T[i + 4] = T[i];
            } else if(cnt == 7) { T[i] = other(T[i - 1], T[j]); T[i + 1] = T[i - 1]; T[i + 2] = T[i]; T[i + 3] = T[j]; T[i + 4] = T[i]; T[i + 5] = T[j]; T[i + 6] = T[i];
            } else if(cnt == 9) { T[i] = other(T[i - 1], T[j]); T[i + 1] = T[i - 1]; T[i + 2] = T[i]; T[i + 3] = T[j]; T[i + 4] = T[i]; T[i + 5] = T[j]; T[i + 6] = T[i];
            } else if(cnt % 2 == 0) {
              for(k = 0; k < cnt; k += 2) {
                T[i + k] = T[j]; T[i + k + 1] = T[i - 1];
              }
            }
          } else {
            cnt = j - i;
            if(cnt == 1) { T[i] = another(T[j]);
            } else if(cnt == 2) { T[i] = another(T[j]); T[i + 1] = other(another(T[j]), T[j]);
            } else if(cnt == 4) { T[i] = another(T[j]); T[i + 1] = other(another(T[j]), T[j]); T[i + 2] = T[j]; T[i + 3] = T[i+1];
            } else if(cnt == 6) { T[i] = another(T[j]); T[i + 1] = other(another(T[j]), T[j]); T[i + 2] = T[i]; T[i + 3] = T[i+1]; T[i + 4] = T[j]; T[i + 5] = T[i + 1];
            } else if(cnt == 8) { T[i] = another(T[j]); T[i + 1] = other(another(T[j]), T[j]); T[i + 2] = T[i]; T[i + 3] = T[i+1]; T[i + 4] = T[j]; T[i + 5] = T[i + 1]; T[i + 6] = T[j]; T[i + 7] = T[i + 1];
            } else if(cnt % 2 == 1) {
              T[i] = another(T[j]);
              for(k = 1; k < cnt; k += 2) {
                T[i + k] = T[j]; T[i + k + 1] = T[i];
              }
            }
          }
          break;
        }
      }
    }
  }

  for(i = N - 1; i > 0; i--) {
    if(T[i] == 'x') {
      if(T[i + 1] != 'a' && T[i + 1] != 'x' && T[i - 1] != 'a') T[i] = 'a';
      else if(T[i + 1] != 'b' && T[i + 1] != 'x' && T[i - 1] != 'b') T[i] = 'b';
      else if(T[i + 1] != 'c' && T[i + 1] != 'x' && T[i - 1] != 'c') T[i] = 'c';
    }
  }

  return 0;
}
