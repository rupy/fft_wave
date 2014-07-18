#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define WAVE_FILE_NAME "test.wav"

typedef struct{
  double re;
  double im;
    } complex;

void PrintBinary(int decimal){//デバッグ用１０進→２進表示

  char binary[100];
  int i=0;
  printf("%d → ",decimal);

  if(decimal==0){//０なら０を表示して終わり
    printf("0\n");
    return;
  }

  while(decimal>0){
    binary[i]=(decimal%2!=0);
    decimal/=2;
    ++i;
  }
  while(i-->0){
    printf("%d",binary[i]);
  }
  printf("\n");
}


void printComplex(complex a){//複素数の値を出力する関数
  printf("re=%f, im=%f\n",a.re,a.im);
}


complex MulComplex(complex x, complex y){
    complex z;
    z.re = x.re * y.re - x.im * y.im;
    z.im = x.re * y.im + y.re * x.im;
    return (z);
}
complex AddComplex(complex x, complex y){
    complex z;
    z.re = x.re + x.re;
    z.im = x.im + y.im;
    return (z);
}
complex SubComplex(complex x, complex y){
    complex z;
    z.re = x.re - x.re;
    z.im = x.im - y.im;
    return (z);
}
complex MinusComplex(complex x){//符号反転
    x.re = -x.re;
    x.im = -x.im;
    return (x);
}
int bitr(int bit, int keta){//ビット反転処理

  int i;
  int bitr;

  bitr = 0;
  for(i=0;i<keta;++i){
    bitr <<= 1;
    bitr |= ( bit & 1 );
    /* if(bit&1){//反転されたビットの２進数表示（デバッグ用）
      printf("1");
    }else{
      printf("0");
      }*/
    bit >>= 1;
  }
  //printf("\n");
  return bitr;
}

void swap(complex* a,complex* b){//２数のスワップ

  double tmp;

  //実部のスワップ
  tmp = a->re;
  a->re = b->re;
  b->re = tmp;

  //虚部のスワップ
  tmp = a->im;
  a->im = b->im;
  b->im = tmp;

}

void sDFT(short x[], int n) // 遅いフーリエ変換
{


	n=8;

  int i, j, max1_i, max2_i;
  double reF, imF, power, max1, max2;

  printf("\n");

  for(i=0;i<n;++i){
    //x[i] = i+1;//デバッグ用：データを単純な数値として計算してみる。
  }

  max1_i = max2_i = 0; // 1 番目と 2 番目に大きい値を持つインデックス
  max1 = max2 = 0.0;
  // 1 番目と 2 番目に大きい値
  for(i = 0; i <= n-1 ; i++) { //X(N-k)={X(k)}^* より半分だけ求める
    reF = imF = 0.0;
    for(j = 0; j < n; j++) {
      reF += x[j] * cos(2 * M_PI * i * j / n);
      imF += -x[j] * sin(2 * M_PI * i * j / n);
    }
     //printf("%d %f %f %f\n", i, reF, imF, reF * reF + imF * imF);
    if( i==0 ) continue;
    power = reF * reF + imF * imF;
    if( power > max1 ){
      max2_i = max1_i; max2 = max1;
      max1_i = i;
      max1 = power;
    }else if( power > max2 ){
      max2_i = i;
      max2 = power;
    }
  }
  printf("%6.1f %6.1f\n", (double)max1_i / n * 44100.0,
	 (double)max2_i / n * 44100.0); // 周波数に変換して表示
}


void sFFT(const short x[], int n) // 高速フーリエ変換(n=分割数,x=変換したい値の離散値)
{

	n=8;
  int i;
  complex* X = ( complex * )malloc(sizeof(complex) * n);

  for(i=0;i<n;++i){//データを複素数に拡張
    X[i].re = (double)x[i];
    //X[i].re = (double)(i+1);//デバッグ用：データを単純な数値として計算してみる。
    X[i].im = 0;//
    //printComplex(X[i]);
  }

  /////////////////////////////////
  //ビット反転処理
  /////////////////////////////////
  int keta = 0;//２進数の桁数
  int tmp_n = n-1;//0〜n-1の値を取るので
  while(tmp_n > 0){//ビット反転処理の為に２進数にｎ-1の２進数の桁数を求める
    tmp_n>>=1;
    ++keta;
  }

  //printf("\n======================\n");
  for(i=0;i<n;i++){//ビットリバース処理
    int b = bitr(i,keta);
    if(i<b){
      swap(&X[i],&X[b]);
    }
    //printf("%d %d (%d)(%d)\n",i,b,n,keta);
    //PrintBinary(b);

    //この処理をすることによって、例えばn=8のときx[1]の代わりにx[4]を得ることができる）
  }

  
  for(i=0;i<n;++i){//デバッグ用（正しくビットが反転されているかの確認）
    //printComplex(X[i]);
  }
  

  /////////////////////////////////
  //高速フーリエ変換
  /////////////////////////////////

  int step,j,k;
  for(step=1;step<n;step*=2){//バタフライ演算の計算ステップ（n=8なら0,2,4の３回）
    for(j=0;j<step;++j){//バタフライ演算の加算値の上段の番号を数える
		complex w;
		w.re = cos( M_PI * j / step);//回転因子wの実部
		w.im = sin( M_PI * j / step);//回転因子wの虚部
      //printf("step=%d j=%d\n",step,j);
      for(k=j;k<n;k+=2*step){//バタフライ演算の加算値の下段の番号を数える
		  
		  complex X0,X1;
		  X0 = X[k];//第一オペランド
		  X1 = MulComplex(w,X[k+step]);//第二オペランド

		  X[k] = AddComplex(X0,X1);//上段の結果
		  X[k+step] = AddComplex(X0,MinusComplex(X1));//下段の結果（半周期回転するのでマイナスが付く）
      }
    }
  }

  for(i=0;i<n;++i){//デバッグ用（結果チェック）
    printComplex(X[i]);
  }

  /////////////////////////////////
  //最大値表示
  /////////////////////////////////

  int max1_i, max2_i;
  double power, max1, max2;

  max1_i = max2_i = 0; // 1 番目と 2 番目に大きい値を持つインデックス
  max1 = max2 = 0.0;  // 1 番目と 2 番目に大きい値

  for(i=0;i<n;++i){
    printf("%d %f %f %f\n",i, X[i].re, X[i].im,X[i].re * X[i].re + X[i].im * X[i].im);

    if( i==0 ) continue;
    power = X[i].re * X[i].re + X[i].im * X[i].im;
    if( power > max1 ){
      max2_i = max1_i; max2 = max1;
      max1_i = i;
      max1 = power;
    }else if( power > max2 ){
      max2_i = i;
      max2 = power;
    }
  }
  printf("%6.1f %6.1f\n", (double)max1_i / n * 44100.0,
	 (double)max2_i / n * 44100.0); // 周波数に変換して表示

  
	free(X);
 
}
int main(int argc, char *argv[])
{

  argc=2;

  int i, filesize, channel, sampling_rate, bit, datasize = 0, st, n2;
  char *buf;
  short *data;
  FILE *fp;
  if( argc!=2 ){ printf("Usage : %s filename\n", argv[0]); exit(1); }
  fp = fopen(WAVE_FILE_NAME, "r");
  if( fp==NULL ){ printf("File not found.\n"); exit(1); }
  fseek(fp,0,SEEK_END);
  // ファイルサイズを調べる
  filesize = ftell(fp);
  rewind(fp);
  // ファイルポインタを先頭に戻す
  buf = (char *)malloc(filesize);
  if( buf==NULL ){ printf("Malloc error.\n"); exit(1); }
  fread(buf, 1, filesize, fp);
  fclose(fp);
  if( strncmp(&buf[0], "RIFF", 4) || strncmp(&buf[8], "WAVEfmt ", 8) ){
    printf("Format error.\n"); exit(1);
  }
  channel = *(short *)&buf[22];
  // チャンネル数
  sampling_rate = *(long *)&buf[24]; // サンプリングレート
  bit = *(short *)&buf[34];
  // ビット数
  data = (short *)&buf[44];
  // 本データ開始位置
  if( (bit != 16) || (sampling_rate != 44100) ){
    printf("Format error.\n"); exit(1);
  }
  printf("Channel = %d\n", channel);
  if( channel == 1 ){
    // モノラル
    datasize = (filesize - 44) / 2;
  }else if( channel == 2 ){ // ステレオ
    data = (short *)&buf[44];
    datasize = (filesize - 44) / 4;
    for(i = 0; i < datasize; i++) data[i] = (data[i*2] + data[i*2+1]) / 2.0;
  }
  printf(" Start (second) Size NumofFFT 1st2nd\n");
  for(i = 0; i < datasize; i++) if( data[i] != 0 ) break; // 0 スキップ
  i--; st=i;
  for(; i < datasize; i++){
    if( data[i] == 0 ){
      // 信号値が 0 の場所を探す
      if( i - st < 8192 ) continue;
      // 短時間の場合は無視
      n2 = 1;
      while(i - st >= n2 * 2) n2 *= 2; // 2 のべき乗になるよう切り捨て
      printf("%7d(%6.2f) %6d %6d ", st , st / 44100.0, i - st, n2);
      fflush(stdout);
      sDFT(&data[st], n2);
      // 遅いフーリエ変換
      sFFT(&data[st], n2);
      // 高速フーリエ変換
      st = i;
      if( st >= 44100 * 30 ) break; // 30 秒の演奏データを解析したら終了
	  break;
    }
  }

  printf("終了\n");
  getchar();
  return 0;
}
