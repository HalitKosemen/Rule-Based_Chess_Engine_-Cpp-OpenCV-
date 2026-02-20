#include <iostream>
#include <opencv4/opencv2/opencv.hpp>
#include <opencv4/opencv2/tracking.hpp>
#include <opencv4/opencv2/imgproc.hpp>
#include <opencv4/opencv2/opencv_modules.hpp>

#include <vector>
#include <string>



cv::Mat beyaz_piyon = cv::imread("/home/halit/chess_project/object/beyaz_piyon.jpeg");
cv::Mat beyaz_kale = cv::imread("/home/halit/chess_project/object/beyaz_kale.jpeg");
cv::Mat beyaz_at = cv::imread("/home/halit/chess_project/object/beyaz_at.jpeg");
cv::Mat beyaz_fil = cv::imread("/home/halit/chess_project/object/beyaz_fil.jpeg");
cv::Mat beyaz_vezir = cv::imread("/home/halit/chess_project/object/beyaz_vezir.jpeg");
cv::Mat beyaz_sah = cv::imread("/home/halit/chess_project/object/beyaz_sah.jpeg");
cv::Mat siyah_piyon = cv::imread("/home/halit/chess_project/object/siyah_piyon.jpeg");
cv::Mat siyah_kale = cv::imread("/home/halit/chess_project/object/siyah_kale.jpeg");
cv::Mat siyah_at = cv::imread("/home/halit/chess_project/object/siyah_at.jpeg");
cv::Mat siyah_fil = cv::imread("/home/halit/chess_project/object/siyah_fil.jpeg");
cv::Mat siyah_vezir = cv::imread("/home/halit/chess_project/object/siyah_vezir.jpeg");
cv::Mat siyah_sah = cv::imread("/home/halit/chess_project/object/siyah_sah.jpeg");




enum class PieceType {NONE , PIYON , KALE , AT, FIL , VEZIR , SAH};
enum class PieceColor {NONE , BEYAZ , SIYAH};

struct Square{
    int index;
    bool hasPiece;
    PieceType pieceType;
    PieceColor pieceColor;
    bool hasMoved;
    Square(int idx):index(idx),hasPiece(false),pieceType(PieceType::NONE),pieceColor(PieceColor::NONE),hasMoved(false){}
};

PieceColor currentTurn = PieceColor::BEYAZ;
std::vector<Square> chessBoard;

std::vector<int> piyonTehlikeKareleri;

std::vector<int> possiableMoves;



PieceType selectedPiece = PieceType::VEZIR;
bool selectionMade = false;
std::vector<cv::Rect> iconRects; 




//GRAFİK GÖRSELLEŞTİRME FONKSİYONLARI

cv::Mat image; //normel chess tahtası görüntüsü 
cv::Mat boardWithGrid; //gridleri çizgileri çekilmiş görsel

int squareSize = 0;

int selected_square = -1;
bool pieceSelected = false;
bool gameOver = false;

cv::Mat drawChessGrid(const cv::Mat& image, int rows = 8, int cols = 8, bool showIndex = false){ //SANTRAÇ TAHTASINDAKİ KARELERİ GÖSTERİYOR
    cv::Mat drawing = image.clone();
    int squareSize = image.rows / rows;

    for(int i = 0; i< rows;i++){
        for(int j = 0;j<cols;j++){
            int x = j * squareSize;
            int y = i * squareSize;

            cv::rectangle(drawing, cv::Point(x, y), cv::Point(x + squareSize, y + squareSize), cv::Scalar(0, 255, 0), 2); 
            if(showIndex){
                std::string idx = std::to_string(i*cols + j);
                cv::putText(drawing, idx, cv::Point(x + 5, y + 20), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0,0,255), 1); 
            }
        }
    }
    return drawing;
}



void placePiece(cv::Mat& board, const cv::Mat& piece, int row, int col){ //BU FONKSİYON GÖRSELLERİ KARELERİN ORTA NOKTALARINA HİZALIYOR
    int squareSize = board.rows / 8;
    int centerX = col * squareSize + squareSize / 2;
    int centerY = row * squareSize + squareSize / 2;

    int pieceCenterX = piece.cols / 2;
    int pieceCenterY = piece.rows / 2;

    // ROI başlangıç noktası
    int x0 = std::max(0,centerX - pieceCenterX);
    int y0 = std::max(0,centerY - pieceCenterY);

    // ROI sınırlarını kontrol et
    int roiWidth = std::min(piece.cols, board.cols - x0);
    int roiHeight = std::min(piece.rows, board.rows - y0);

    cv::Mat roi = board(cv::Rect(x0, y0, roiWidth, roiHeight));
    cv::Mat pieceROI = piece(cv::Rect(0, 0, roiWidth, roiHeight));

    if(piece.channels() == 4){
        // Alpha blending
        for(int y = 0; y < roiHeight; y++){
            for(int x = 0; x < roiWidth; x++){
                cv::Vec4b px = pieceROI.at<cv::Vec4b>(y, x);
                float alpha = px[3] / 255.0f;
                for(int c = 0; c < 3; c++){
                    roi.at<cv::Vec3b>(y, x)[c] =
                        alpha * px[c] + (1 - alpha) * roi.at<cv::Vec3b>(y, x)[c];
                }
            }
        }
    } else {
        // BGR görsel ise direkt kopyala
        pieceROI.copyTo(roi);
    }
}

void placePieceIndex(cv::Mat& board,const cv::Mat& Piece, int squareIndex){ //İSTENİLEN İNDEXTEKİ KAREYE RESMİ YAPIŞTIRIYOR
    int row = squareIndex / 8;
    int col = squareIndex % 8;
    placePiece(board,Piece,row,col);
}

void firstChessBoard(){ //GÖRSEL OLARAK İLK BAŞLARKEN TAŞLARIN YERLERİNE YERLEŞTİRİLMESİ 
    placePieceIndex(boardWithGrid,siyah_kale,0);
    placePieceIndex(boardWithGrid,siyah_at,1);
    placePieceIndex(boardWithGrid,siyah_fil,2);
    placePieceIndex(boardWithGrid,siyah_vezir,3);
    placePieceIndex(boardWithGrid,siyah_sah,4);
    placePieceIndex(boardWithGrid,siyah_fil,5);
    placePieceIndex(boardWithGrid,siyah_at,6);
    placePieceIndex(boardWithGrid,siyah_kale,7);

    placePieceIndex(boardWithGrid,siyah_piyon,8);
    placePieceIndex(boardWithGrid,siyah_piyon,9);
    placePieceIndex(boardWithGrid,siyah_piyon,10);
    placePieceIndex(boardWithGrid,siyah_piyon,11);
    placePieceIndex(boardWithGrid,siyah_piyon,12);
    placePieceIndex(boardWithGrid,siyah_piyon,13);
    placePieceIndex(boardWithGrid,siyah_piyon,14);
    placePieceIndex(boardWithGrid,siyah_piyon,15);

    placePieceIndex(boardWithGrid,beyaz_kale,56);
    placePieceIndex(boardWithGrid,beyaz_at,57);
    placePieceIndex(boardWithGrid,beyaz_fil,58);
    placePieceIndex(boardWithGrid,beyaz_vezir,59);
    placePieceIndex(boardWithGrid,beyaz_sah,60);
    placePieceIndex(boardWithGrid,beyaz_fil,61);
    placePieceIndex(boardWithGrid,beyaz_at,62);
    placePieceIndex(boardWithGrid,beyaz_kale,63);

    placePieceIndex(boardWithGrid,beyaz_piyon,48);
    placePieceIndex(boardWithGrid,beyaz_piyon,49);
    placePieceIndex(boardWithGrid,beyaz_piyon,50);
    placePieceIndex(boardWithGrid,beyaz_piyon,51);
    placePieceIndex(boardWithGrid,beyaz_piyon,52);
    placePieceIndex(boardWithGrid,beyaz_piyon,53);
    placePieceIndex(boardWithGrid,beyaz_piyon,54);
    placePieceIndex(boardWithGrid,beyaz_piyon,55);

}

void updateVisualBoard() {
    boardWithGrid = drawChessGrid(image); // gridleri çiz
    for(const auto& sq : chessBoard){
        if(!sq.hasPiece) continue;
        cv::Mat pieceImage;
        switch(sq.pieceType){
            case PieceType::PIYON: pieceImage = (sq.pieceColor == PieceColor::BEYAZ) ? beyaz_piyon : siyah_piyon; break;
            case PieceType::KALE: pieceImage = (sq.pieceColor == PieceColor::BEYAZ) ? beyaz_kale : siyah_kale; break;
            case PieceType::AT: pieceImage = (sq.pieceColor == PieceColor::BEYAZ) ? beyaz_at : siyah_at; break;
            case PieceType::FIL: pieceImage = (sq.pieceColor == PieceColor::BEYAZ) ? beyaz_fil : siyah_fil; break;
            case PieceType::VEZIR: pieceImage = (sq.pieceColor == PieceColor::BEYAZ) ? beyaz_vezir : siyah_vezir; break;
            case PieceType::SAH: pieceImage = (sq.pieceColor == PieceColor::BEYAZ) ? beyaz_sah : siyah_sah; break;
            default: continue;
        }
        // sq.index sadece burada kullanılabilir
        placePieceIndex(boardWithGrid, pieceImage, sq.index);
    }
}

void printBoardInfo() {
    for(const auto& sq : chessBoard){
        std::cout << "Kare " << sq.index << ": ";
        if(sq.hasPiece){
            std::string type;
            switch(sq.pieceType){
                case PieceType::PIYON: type = "Piyon"; break;
                case PieceType::KALE: type = "Kale"; break;
                case PieceType::AT: type = "At"; break;
                case PieceType::FIL: type = "Fil"; break;
                case PieceType::VEZIR: type = "Vezir"; break;
                case PieceType::SAH: type = "Sah"; break;
                default: type = "Unknown";
            }
            std::string color = (sq.pieceColor == PieceColor::BEYAZ) ? "Beyaz" : "Siyah";
            std::cout << color << " " << type << std::endl;
        } else {
            std::cout << "Boş" << std::endl;
        }
    }
}


void firstChessInitalize(){

    chessBoard.clear();
    for(int i = 0;i < 64 ; i++) chessBoard.push_back(Square(i));

    chessBoard[0].hasPiece = true;
    chessBoard[0].pieceType = PieceType::KALE;
    chessBoard[0].pieceColor = PieceColor::SIYAH;

    chessBoard[1].hasPiece = true;
    chessBoard[1].pieceType = PieceType::AT;
    chessBoard[1].pieceColor = PieceColor::SIYAH;
    
    chessBoard[2].hasPiece = true;
    chessBoard[2].pieceType = PieceType::FIL;
    chessBoard[2].pieceColor = PieceColor::SIYAH;

    chessBoard[3].hasPiece = true;
    chessBoard[3].pieceType = PieceType::VEZIR;
    chessBoard[3].pieceColor = PieceColor::SIYAH;

    chessBoard[4].hasPiece = true;
    chessBoard[4].pieceType = PieceType::SAH;
    chessBoard[4].pieceColor = PieceColor::SIYAH;

    chessBoard[5].hasPiece = true;
    chessBoard[5].pieceType = PieceType::FIL;
    chessBoard[5].pieceColor = PieceColor::SIYAH;

    chessBoard[6].hasPiece = true;
    chessBoard[6].pieceType = PieceType::AT;
    chessBoard[6].pieceColor = PieceColor::SIYAH;

    chessBoard[7].hasPiece = true;
    chessBoard[7].pieceType = PieceType::KALE;
    chessBoard[7].pieceColor = PieceColor::SIYAH;

    chessBoard[8].hasPiece = true;
    chessBoard[8].pieceType = PieceType::PIYON;
    chessBoard[8].pieceColor = PieceColor::SIYAH;

    chessBoard[9].hasPiece = true;
    chessBoard[9].pieceType = PieceType::PIYON;
    chessBoard[9].pieceColor = PieceColor::SIYAH;

    chessBoard[10].hasPiece = true;
    chessBoard[10].pieceType = PieceType::PIYON;
    chessBoard[10].pieceColor = PieceColor::SIYAH;

    chessBoard[11].hasPiece = true;
    chessBoard[11].pieceType = PieceType::PIYON;
    chessBoard[11].pieceColor = PieceColor::SIYAH;

    chessBoard[12].hasPiece = true;
    chessBoard[12].pieceType = PieceType::PIYON;
    chessBoard[12].pieceColor = PieceColor::SIYAH;

    chessBoard[13].hasPiece = true;
    chessBoard[13].pieceType = PieceType::PIYON;
    chessBoard[13].pieceColor = PieceColor::SIYAH;

    chessBoard[14].hasPiece = true;
    chessBoard[14].pieceType = PieceType::PIYON;
    chessBoard[14].pieceColor = PieceColor::SIYAH;

    chessBoard[15].hasPiece = true;
    chessBoard[15].pieceType = PieceType::PIYON;
    chessBoard[15].pieceColor = PieceColor::SIYAH;



    chessBoard[48].hasPiece = true;
    chessBoard[48].pieceType = PieceType::PIYON;
    chessBoard[48].pieceColor = PieceColor::BEYAZ;

    chessBoard[49].hasPiece = true;
    chessBoard[49].pieceType = PieceType::PIYON;
    chessBoard[49].pieceColor = PieceColor::BEYAZ;

    chessBoard[50].hasPiece = true;
    chessBoard[50].pieceType = PieceType::PIYON;
    chessBoard[50].pieceColor = PieceColor::BEYAZ;

    chessBoard[51].hasPiece = true;
    chessBoard[51].pieceType = PieceType::PIYON;
    chessBoard[51].pieceColor = PieceColor::BEYAZ;

    chessBoard[52].hasPiece = true;
    chessBoard[52].pieceType = PieceType::PIYON;
    chessBoard[52].pieceColor = PieceColor::BEYAZ;

    chessBoard[53].hasPiece = true;
    chessBoard[53].pieceType = PieceType::PIYON;
    chessBoard[53].pieceColor = PieceColor::BEYAZ;

    chessBoard[54].hasPiece = true;
    chessBoard[54].pieceType = PieceType::PIYON;
    chessBoard[54].pieceColor = PieceColor::BEYAZ;

    chessBoard[55].hasPiece = true;
    chessBoard[55].pieceType = PieceType::PIYON;
    chessBoard[55].pieceColor = PieceColor::BEYAZ;

    chessBoard[56].hasPiece = true;
    chessBoard[56].pieceType = PieceType::KALE;
    chessBoard[56].pieceColor = PieceColor::BEYAZ;

    chessBoard[57].hasPiece = true;
    chessBoard[57].pieceType = PieceType::AT;
    chessBoard[57].pieceColor = PieceColor::BEYAZ;

    chessBoard[58].hasPiece = true;
    chessBoard[58].pieceType = PieceType::FIL;
    chessBoard[58].pieceColor = PieceColor::BEYAZ;

    chessBoard[59].hasPiece = true;
    chessBoard[59].pieceType = PieceType::VEZIR;
    chessBoard[59].pieceColor = PieceColor::BEYAZ;

    chessBoard[60].hasPiece = true;
    chessBoard[60].pieceType = PieceType::SAH;
    chessBoard[60].pieceColor = PieceColor::BEYAZ;

    chessBoard[61].hasPiece = true;
    chessBoard[61].pieceType = PieceType::FIL;
    chessBoard[61].pieceColor = PieceColor::BEYAZ;

    chessBoard[62].hasPiece = true;
    chessBoard[62].pieceType = PieceType::AT;
    chessBoard[62].pieceColor = PieceColor::BEYAZ;

    chessBoard[63].hasPiece = true;
    chessBoard[63].pieceType = PieceType::KALE;
    chessBoard[63].pieceColor = PieceColor::BEYAZ;

}




std::vector<int> piyonSolidMoves(int index , const std::vector<Square>& board){
    std::vector<int> moves;
    if (index < 0 || index >= 64) return moves;

    const Square& sq = board[index];
    if (!sq.hasPiece || sq.pieceType != PieceType::PIYON) return moves;

    int row = index / 8;
    int col = index % 8;

    int direction = (sq.pieceColor == PieceColor::BEYAZ) ? -1 : 1;
    int nextRow = row + direction;

    if (nextRow >= 0 && nextRow < 8) {
        int forward = nextRow * 8 + col;
        if (!board[forward].hasPiece) {
            moves.push_back(forward);

            // 2 kare ileri (sadece ilk sıradaysa)
            bool isStartRow = (sq.pieceColor == PieceColor::BEYAZ && row == 6) ||
                              (sq.pieceColor == PieceColor::SIYAH && row == 1);
            if (isStartRow) {
                int twoForward = (row + 2 * direction) * 8 + col;
                if (!board[twoForward].hasPiece)
                    moves.push_back(twoForward);
            }
        }
    }

    int diagLeftCol = col - 1;
    if (nextRow >= 0 && nextRow < 8 && diagLeftCol >= 0) {
        int diagLeft = nextRow * 8 + diagLeftCol;
        if (board[diagLeft].hasPiece && board[diagLeft].pieceColor != sq.pieceColor)
            moves.push_back(diagLeft);
    }

    int diagRightCol = col + 1;
    if (nextRow >= 0 && nextRow < 8 && diagRightCol < 8) {
        int diagRight = nextRow * 8 + diagRightCol;
        if (board[diagRight].hasPiece && board[diagRight].pieceColor != sq.pieceColor)
            moves.push_back(diagRight);
    }

    for(int &m : moves){
        int targetRow = m / 8;
        if((sq.pieceColor == PieceColor::BEYAZ && targetRow == 0) || (sq.pieceColor == PieceColor::SIYAH && targetRow == 7)){
            std::cout << "Piyon Terfi!" << m << std::endl;
        }
    }
    return moves;
}



std::vector<int> filSolidMoves(int index, const std::vector<Square>& board){
    std::vector<int> moves;
    if(index < 0 || index >= 64) return moves;

    const Square& sq = board[index];
    if(!sq.hasPiece || sq.pieceType != PieceType::FIL) return moves;

    int row = index / 8;
    int col = index % 8;

    const int dirs[4][2] = { {-1,-1}, {-1,1}, {1,-1}, {1,1} };

    for(int d = 0; d < 4; ++d){
        int dr = dirs[d][0];
        int dc = dirs[d][1];
        int r = row + dr;
        int c = col + dc;

        while(r >= 0 && r < 8 && c >=0 && c < 8){
            int targetIndex = r * 8 + c;
            const Square& target = board[targetIndex];

            if(!target.hasPiece){
                moves.push_back(targetIndex);
            }else{
                if(target.pieceColor != sq.pieceColor){
                    moves.push_back(targetIndex);
                }
                break;
            }
            r += dr;
            c += dc;
        }
    }
    return moves;
}


std::vector<int> kaleSolidMoves(int index, const std::vector<Square>& board){
    std::vector<int> moves;
    if (index < 0 || index >= 64) return moves;
    const Square& sq = board[index];
    if (!sq.hasPiece || sq.pieceType != PieceType::KALE) return moves;

    int row = index / 8;
    int col = index % 8;

    const int dirs[4][2] = { {-1,0}, {1,0}, {0,-1}, {0,1} };

    for(int d = 0; d < 4;++d){
        int dr = dirs[d][0];
        int dc = dirs[d][1];
        int r = row + dr;
        int c = col + dc;

        while (r >= 0 && r < 8 && c >= 0 && c <8){
            int targetIndex = r * 8 + c;
            const Square& target = board[targetIndex];

            if(!target.hasPiece){
                moves.push_back(targetIndex);
            }else{
                if(target.pieceColor != sq.pieceColor){
                    moves.push_back(targetIndex);
                }
                break;
            }
            r += dr;
            c += dc;
        }
    }
    return moves;
}


std::vector<int> atSolidMoves(int index, const std::vector<Square>& board){
    std::vector<int> moves;
    if(index < 0 || index >= 64) return moves;

    const Square& sq = board[index];
    if(!sq.hasPiece || sq.pieceType != PieceType::AT) return moves;

    int row = index / 8;
    int col = index % 8;

    const int dr[8] = {-2, -2, -1, -1, 1, 1, 2, 2};
    const int dc[8] = {-1, 1, -2, 2, -2, 2, -1, 1};

    for(int i = 0; i<8;++i){
        int r = row + dr[i];
        int c = col + dc[i];

        if(r >= 0 && r < 8 && c>= 0 && c < 8){
            int targetIndex = r * 8 + c;
            const Square& target = board[targetIndex];

            if(!target.hasPiece || target.pieceColor != sq.pieceColor){
                moves.push_back(targetIndex);
            }
        }
    }
    return moves;
}

std::vector<int> vezirSolidMoves(int index, const std::vector<Square>& board){
    std::vector<int> moves;
    if(index < 0 || index >= 64) return moves;

     const Square& sq = board[index];
     if(!sq.hasPiece || sq.pieceType != PieceType::VEZIR) return moves;

    int row = index / 8;
    int col = index % 8;

    const int dirs[8][2] = {
        {-1,0}, {1,0}, {0,-1}, {0,1},    // dikey ve yatay (kale)
        {-1,-1}, {-1,1}, {1,-1}, {1,1}   // çapraz (fil)
    };

    for(int d=0;d<8;++d){
        int dr = dirs[d][0];
        int dc = dirs[d][1];
        int r = row + dr;
        int c = col + dc;

        while(r>=0 && r < 8 && c>=0 && c<8){
            int targetIndex = r * 8 + c;
            const Square& target = board[targetIndex];

            if(!target.hasPiece){
                moves.push_back(targetIndex);
            }else{
                if(target.pieceColor != sq.pieceColor){
                    moves.push_back(targetIndex);
                }
                break;
            }
            r += dr;
            c += dc;
        }
    }
    return moves;
}


std::vector<int> piyonAttackSquares(int index, PieceColor color, const std::vector<Square>& board){
    std::vector<int> attacks;
    int row = index / 8;
    int col = index % 8;

    int dir = (color == PieceColor::BEYAZ) ? -1 : 1; // beyaz yukarı, siyah aşağı

    // çapraz sol
    if(col > 0 && row+dir >=0 && row+dir < 8)
        attacks.push_back((row+dir)*8 + (col-1));
    // çapraz sağ
    if(col < 7 && row+dir >=0 && row+dir < 8)
        attacks.push_back((row+dir)*8 + (col+1));

    return attacks;
}



bool isSquareAttacked(int targetIndex, PieceColor byColor, const std::vector<Square>& board) {
    for (int i = 0; i < 64; ++i) {
        const Square& sq = board[i];
        if (!sq.hasPiece || sq.pieceColor != byColor) continue;

        std::vector<int> attacks;

        switch (sq.pieceType) {
            case PieceType::PIYON:
                attacks = piyonAttackSquares(i, sq.pieceColor, board);
                break;

            case PieceType::KALE:
                attacks = kaleSolidMoves(i, board);
                break;

            case PieceType::AT:
                attacks = atSolidMoves(i, board);
                break;

            case PieceType::FIL:
                attacks = filSolidMoves(i, board);
                break;

            case PieceType::VEZIR:
                attacks = vezirSolidMoves(i, board);
                break;

            case PieceType::SAH: {
                const int dr[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
                const int dc[8] = {-1, 0, 1, -1, 1, -1, 0, 1};
                int row = i / 8, col = i % 8;

                for (int k = 0; k < 8; ++k) {
                    int r = row + dr[k];
                    int c = col + dc[k];
                    if (r >= 0 && r < 8 && c >= 0 && c < 8)
                        attacks.push_back(r * 8 + c);
                }
                break;  // <-- şah için break burada olmalı
            }

            default:
                break;
        }

        // switch dışına çıktıktan sonra kontrol yapılır
        for (int a : attacks) {
            if (a == targetIndex)
                return true;
        }
    }
    return false;
}

bool isSahInCheck(const std::vector<Square>& board, PieceColor color) {
    int sahIndex = -1;

    // 1️⃣ Önce ilgili renkteki şahın karesini bul
    for (const auto& sq : board) {
        if (sq.hasPiece && sq.pieceType == PieceType::SAH && sq.pieceColor == color) {
            sahIndex = sq.index;
            break;
        }
    }

    // Şah bulunamadıysa hata durumu (örneğin tahta hatalı)
    if (sahIndex == -1) return false;

    // 2️⃣ Rakip rengini belirle
    PieceColor enemyColor = (color == PieceColor::BEYAZ) ? PieceColor::SIYAH : PieceColor::BEYAZ;

    // 3️⃣ Şahın bulunduğu kare rakip tarafından saldırı altında mı?
    return isSquareAttacked(sahIndex, enemyColor, board);
}




std::vector<int> sahSolidMoves(int index, const std::vector<Square>& board) {
    std::vector<int> moves;
    if (index < 0 || index >= 64) return moves;

    const Square& sq = board[index];
    if (!sq.hasPiece || sq.pieceType != PieceType::SAH) return moves;

    PieceColor color = sq.pieceColor;
    PieceColor opponent = (color == PieceColor::BEYAZ) ? PieceColor::SIYAH : PieceColor::BEYAZ;

    // Rakip şahın konumunu bul
    int opponentKingIndex = -1;
    for (int i = 0; i < 64; ++i) {
        if (board[i].hasPiece && board[i].pieceType == PieceType::SAH && board[i].pieceColor == opponent) {
            opponentKingIndex = i;
            break;
        }
    }

    int oppRow = (opponentKingIndex == -1) ? -100 : (opponentKingIndex / 8);
    int oppCol = (opponentKingIndex == -1) ? -100 : (opponentKingIndex % 8);

    const int dr[8] = {-1,-1,-1,0,0,1,1,1};
    const int dc[8] = {-1,0,1,-1,1,-1,0,1};

    int row = index / 8;
    int col = index % 8;

    for (int k = 0; k < 8; ++k) {
        int r = row + dr[k];
        int c = col + dc[k];
        if (r < 0 || r >= 8 || c < 0 || c >= 8) continue;

        int targetIndex = r * 8 + c;
        const Square& targetSq = board[targetIndex];

        // 1️⃣ Kendi taşının üstüne gidemez
        if (targetSq.hasPiece && targetSq.pieceColor == color)
            continue;

        // 2️⃣ İki şah yan yana olamaz
        if (opponentKingIndex != -1) {
            if (abs(r - oppRow) <= 1 && abs(c - oppCol) <= 1)
                continue;
        }

        // 3️⃣ Şahın oraya gidince tehdit altında kalıp kalmadığını kontrol et
        std::vector<Square> temp = board; // küçük kopya (sadece 64 eleman)

        // Taşı hareket ettir
        temp[targetIndex] = temp[index];
        temp[targetIndex].index = targetIndex;

        // Eski kareyi boşalt
        temp[index] = Square(index);
        temp[index].hasPiece = false;

        // Eğer yeni kare rakip tarafından tehdit ediliyorsa, gidemez
        if (isSquareAttacked(targetIndex, opponent, temp))
            continue;

        // ✅ Hamle geçerli
        moves.push_back(targetIndex);
    }
    if(!sq.hasMoved && !isSahInCheck(board,color)){
        int kaleSideRookIndex = row * 8 + 7;
        const Square& kaleSideRook = board[kaleSideRookIndex];
        if(kaleSideRook.hasPiece && kaleSideRook.pieceType == PieceType::KALE && kaleSideRook.pieceColor == color && !kaleSideRook.hasMoved){
            if(!board[row * 8 + 5].hasPiece && !board[row * 8 + 6].hasPiece){
                bool safe = true;
                std::vector<Square> tempBoard = board;
                for(int sqIdx : {row * 8 + 5, row * 8 + 6}){
                    tempBoard[sqIdx] = tempBoard[index];
                    tempBoard[sqIdx].index = sqIdx;
                    tempBoard[index] = Square(index);
                    if(isSquareAttacked(sqIdx,opponent,tempBoard)){
                        safe = false;
                        break;
                    }
                    tempBoard = board;
                }
                if(safe){
                    moves.push_back(row * 8 + 6);
                }
            }
        }
        int sahSideRookIndex = row * 8;
        const Square& sahSideRook = board[sahSideRookIndex];
        if(sahSideRook.hasPiece && sahSideRook.pieceType == PieceType::KALE && sahSideRook.pieceColor == color && !sahSideRook.hasMoved){
            if(!board[row * 8 + 1].hasPiece && !board[row * 8 + 2].hasPiece && !board[row * 8 + 3].hasPiece){
                bool safe = true;
                std::vector<Square> tempBoard = board;
                for(int sqIdx : {row * 8 + 2, row *8 +3}){
                    tempBoard[sqIdx] = tempBoard[index];
                    tempBoard[sqIdx].index = sqIdx;
                    tempBoard[index] = Square(index);
                    if(isSquareAttacked(sqIdx,opponent,tempBoard)){
                        safe = false;
                        break;
                    }
                    tempBoard = board;
                }
                if(safe){
                    moves.push_back(row * 8 + 2);
                }
            }
        }
    }

    return moves;
}


std::vector<int> allWhiteAttackSquares(const std::vector<Square>& board) {
    std::vector<int> allAttacks;

    for (int i = 0; i < 64; ++i) {
        const Square& sq = board[i];
        if (!sq.hasPiece || sq.pieceColor != PieceColor::BEYAZ)
            continue;

        std::vector<int> attacks;

        switch (sq.pieceType) {
            case PieceType::PIYON:
                attacks = piyonAttackSquares(i, PieceColor::BEYAZ, board);
                break;
            case PieceType::KALE:
                attacks = kaleSolidMoves(i, board);
                break;
            case PieceType::AT:
                attacks = atSolidMoves(i, board);
                break;
            case PieceType::FIL:
                attacks = filSolidMoves(i, board);
                break;
            case PieceType::VEZIR:
                attacks = vezirSolidMoves(i, board);
                break;
            case PieceType::SAH: {
                // Şah sadece çevresindeki 8 kareyi tehdit eder
                const int dr[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
                const int dc[8] = {-1, 0, 1, -1, 1, -1, 0, 1};
                int row = i / 8, col = i % 8;
                for (int k = 0; k < 8; ++k) {
                    int r = row + dr[k];
                    int c = col + dc[k];
                    if (r >= 0 && r < 8 && c >= 0 && c < 8)
                        attacks.push_back(r * 8 + c);
                }
                break;
            }
            default:
                break;
        }

        allAttacks.insert(allAttacks.end(), attacks.begin(), attacks.end());
    }

    // 🔁 Tekrarlanan kareleri sil
    std::sort(allAttacks.begin(), allAttacks.end());
    allAttacks.erase(std::unique(allAttacks.begin(), allAttacks.end()), allAttacks.end());

    return allAttacks;
}


void checkPawnPromotion(std::vector<Square>& board, int movedIndex) {
    Square& sq = board[movedIndex];
    if (sq.pieceType != PieceType::PIYON) return;

    int row = movedIndex / 8;
    if (!((sq.pieceColor == PieceColor::BEYAZ && row == 0) ||
          (sq.pieceColor == PieceColor::SIYAH && row == 7))) return;

    std::vector<cv::Mat> images = (sq.pieceColor == PieceColor::BEYAZ) ?
        std::vector<cv::Mat>{beyaz_vezir, beyaz_kale, beyaz_fil, beyaz_at} :
        std::vector<cv::Mat>{siyah_vezir, siyah_kale, siyah_fil, siyah_at};

    for (auto& img : images) {
        if (img.empty()) {
            std::cerr << "Hata: Terfi görüntüsü yüklenemedi." << std::endl;
            sq.pieceType = PieceType::VEZIR;
            return;
        }
    }

    const int iconSize = 60;
    const int margin = 20;
    int canvasHeight = images.size() * iconSize + (images.size() + 1) * margin;
    int canvasWidth = iconSize + 2 * margin;
    cv::Mat canvas(canvasHeight, canvasWidth, CV_8UC3, cv::Scalar(50, 50, 50));

    std::vector<cv::Rect> iconRects;
    for (size_t i = 0; i < images.size(); ++i) {
        int y = margin + i * (iconSize + margin);
        int x = margin;
        images[i].copyTo(canvas(cv::Rect(x, y, iconSize, iconSize)));
        iconRects.push_back(cv::Rect(x, y, iconSize, iconSize));

        std::string label;
        switch (i) {
            case 0: label = "Vezir"; break;
            case 1: label = "Kale"; break;
            case 2: label = "Fil"; break;
            case 3: label = "At"; break;
        }
        cv::putText(canvas, label, cv::Point(x + iconSize + 5, y + iconSize / 2 + 5),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
    }

    // Seçim için static değişkenler (callback tarafından erişilebilir)
    static PieceType selectedPiece = PieceType::VEZIR;
    static bool selectionMade = false;

    struct MouseData {
        std::vector<cv::Rect>* rects;
        PieceType* selectedPiece;
        bool* selectionMade;
    };
    MouseData* mouseData = new MouseData{ &iconRects, &selectedPiece, &selectionMade };

    cv::namedWindow("Piyon Terfi Secimi", cv::WINDOW_AUTOSIZE);

    cv::setMouseCallback("Piyon Terfi Secimi",
        [](int event, int x, int y, int, void* userdata) {
            if (event != cv::EVENT_LBUTTONDOWN) return;
            MouseData* data = static_cast<MouseData*>(userdata);
            for (size_t i = 0; i < data->rects->size(); ++i) {
                if (data->rects->at(i).contains(cv::Point(x, y))) {
                    switch (i) {
                        case 0: *(data->selectedPiece) = PieceType::VEZIR; break;
                        case 1: *(data->selectedPiece) = PieceType::KALE; break;
                        case 2: *(data->selectedPiece) = PieceType::FIL; break;
                        case 3: *(data->selectedPiece) = PieceType::AT; break;
                    }
                    *(data->selectionMade) = true;
                }
            }
        },
        mouseData
    );

    selectionMade = false;
    selectedPiece = PieceType::VEZIR;

    while (!selectionMade) {
        cv::imshow("Piyon Terfi Secimi", canvas);
        if (cv::waitKey(30) == 27) { // ESC tuşu ile Vezir seç
            selectedPiece = PieceType::VEZIR;
            break;
        }
    }

    // Tahtayı güncelle
    sq.pieceType = selectedPiece;
    updateVisualBoard();
    cv::imshow("Chess", boardWithGrid);

    cv::destroyWindow("Piyon Terfi Secimi");
    delete mouseData;

    // Konsola yazdır
    std::cout << "Piyon terfi etti -> ";
    switch (sq.pieceType) {
        case PieceType::VEZIR: std::cout << "Vezir"; break;
        case PieceType::KALE: std::cout << "Kale"; break;
        case PieceType::FIL: std::cout << "Fil"; break;
        case PieceType::AT: std::cout << "At"; break;
        default: std::cout << "Bilinmeyen"; break;
    }
    std::cout << std::endl;
}







void movePieceOnFunction(int fromIndex , int toIndex){ //BİR TAŞI BAŞKA BİR KAREYE İLERLETMEK İÇİN KULLANILIYOR 
    chessBoard[toIndex].hasPiece = true;
    chessBoard[toIndex].pieceType = chessBoard[fromIndex].pieceType;
    chessBoard[toIndex].pieceColor = chessBoard[fromIndex].pieceColor;
    chessBoard[toIndex].hasMoved = true;

    chessBoard[fromIndex].hasPiece = false;
    chessBoard[fromIndex].pieceType = PieceType::NONE;
    chessBoard[fromIndex].pieceColor = PieceColor::NONE;

    if(chessBoard[toIndex].pieceType == PieceType::SAH){
        int row = toIndex / 8;
        int col = toIndex % 8;
        if(col == 6){
            int rookFrom = row * 8 + 7;
            int rookTo = row * 8 + 5;
            chessBoard[rookTo] = chessBoard[rookFrom];
            chessBoard[rookTo].index = rookTo;
            chessBoard[rookTo].hasMoved = true;
            chessBoard[rookFrom] = Square(rookFrom);
        }else if(col == 2){
            int rookFrom = row * 8 + 0;
            int rookTo = row * 8 + 3;
            chessBoard[rookTo] = chessBoard[rookFrom];
            chessBoard[rookTo].index = rookTo;
            chessBoard[rookTo].hasMoved = true;
            chessBoard[rookFrom] = Square(rookFrom);
        }
    }
    
    currentTurn = (currentTurn == PieceColor::BEYAZ) ? PieceColor::SIYAH : PieceColor::BEYAZ;
}



void onMouse(int event,int x ,int y, int flags, void* userdata){
    if(gameOver) return;
    if(event == cv::EVENT_RBUTTONDOWN){
        selected_square = -1;
    }
    if(event == cv::EVENT_LBUTTONDOWN){
        int row = y / squareSize;
        int col = x / squareSize;
        int squareIndex = row * 8 + col;
        if (squareIndex < 0 || squareIndex >= 64) return;
        std::cout << "Tiklanan kare: " << squareIndex << " (r:" << row << " c:" << col << ")" << std::endl;

        if(selected_square == -1){
            if(chessBoard[squareIndex].hasPiece){
                if(chessBoard[squareIndex].pieceColor != currentTurn){
                    std::cout << "Bu taş şu an oynayamaz. Sira: " << ((currentTurn==PieceColor::BEYAZ) ? "Beyaz" : "Siyah") << std::endl;
                    return;
                }
                selected_square = squareIndex;

                if(chessBoard[squareIndex].pieceType == PieceType::PIYON){
                    possiableMoves = piyonSolidMoves(squareIndex,chessBoard);
                }else if(chessBoard[squareIndex].pieceType == PieceType::KALE){
                    possiableMoves = kaleSolidMoves(squareIndex,chessBoard);
                }else if(chessBoard[squareIndex].pieceType == PieceType::AT){
                    possiableMoves = atSolidMoves(squareIndex,chessBoard);
                }else if(chessBoard[squareIndex].pieceType == PieceType::FIL){
                    possiableMoves = filSolidMoves(squareIndex,chessBoard);
                }else if(chessBoard[squareIndex].pieceType == PieceType::VEZIR){
                    possiableMoves = vezirSolidMoves(squareIndex,chessBoard);
                }else if(chessBoard[squareIndex].pieceType == PieceType::SAH){
                    possiableMoves = sahSolidMoves(squareIndex,chessBoard);
                }

                if(isSahInCheck(chessBoard,currentTurn)){
                    std::vector<int> filterMoves;
                    std::vector<int> rescuingPieces;
                    for(int m : possiableMoves){
                        std::vector<Square> tempBoard = chessBoard;
                        tempBoard[m] = tempBoard[selected_square];
                        tempBoard[m].index = m;
                        tempBoard[selected_square] = Square(selected_square);
                        if(!isSahInCheck(tempBoard,currentTurn)){
                            filterMoves.push_back(m);
                            rescuingPieces.push_back(selected_square);
                        }
                    }
                    possiableMoves = filterMoves;

                    std::vector<int> allRescuingPieces;
                    for(int i = 0;i<64;i++){
                        if(chessBoard[i].hasPiece && chessBoard[i].pieceColor == currentTurn){
                            std::vector<int> moves;
                            switch(chessBoard[i].pieceType){
                                case PieceType::PIYON: moves = piyonSolidMoves(i,chessBoard); break;
                                case PieceType::KALE: moves = kaleSolidMoves(i,chessBoard); break;
                                case PieceType::AT: moves = atSolidMoves(i,chessBoard); break;
                                case PieceType::FIL: moves = filSolidMoves(i,chessBoard); break;
                                case PieceType::VEZIR: moves = vezirSolidMoves(i,chessBoard); break;
                                case PieceType::SAH: moves = sahSolidMoves(i,chessBoard); break;
                                default: break;
                            }
                            for(int m : moves){
                                std::vector<Square> tempBoard = chessBoard;
                                tempBoard[m] = tempBoard[i];
                                tempBoard[m].index = m;
                                tempBoard[i] = Square(i);
                                if(!isSahInCheck(tempBoard,currentTurn)){
                                    allRescuingPieces.push_back(i);
                                    break;
                                }
                            }
                        }
                    }
                    if(allRescuingPieces.empty()){
                        std::cout << "MAT! " << ((currentTurn==PieceColor::BEYAZ) ? "Beyaz" : "Siyah") << " kaybetti!" << std::endl;
                        gameOver = true;
                        cv::namedWindow("MAT",cv::WINDOW_NORMAL);
                        return;
                    }
                    if(!allRescuingPieces.empty()){
                        std::cout << "Şahi tehditten kurtarabilecek taşlar (tüm taşlar tarandı): ";
                        for(int idx : allRescuingPieces) std::cout << idx << " ";
                        std::cout << std::endl;
                    }
                    if(!rescuingPieces.empty()){
                        std::cout << "Şahi tehditten kurtarabilecek taşlar: ";
                        for(int idx : rescuingPieces) std::cout << idx << " ";
                        std::cout << std::endl;
                    }
                    if(possiableMoves.empty()){
                        std::cout << "Şah tehdit altinda! Bu taşla şahi tehditten kurtaramazsiniz, başka bir taş seçin!" << std::endl;
                        selected_square = -1;
                        return;
                    }else{
                        std::cout << "Şah durumunda geçerli hamleler: ";
                        for(int m : possiableMoves) std::cout << m << " ";
                        std::cout << std::endl;
                    }
                }

                if(possiableMoves.empty()){
                    selected_square = -1;
                    std::cout << "Bu taşin hareket edebileceği kare yok!" << std::endl;
                    return;
                }
                
                std::cout << "Seçilen kare: " << selected_square << std::endl;
                std::cout << "Oynayabileceği kareler: ";
                for(int m : possiableMoves) std::cout << m << " ";
                std::cout << std::endl;
            }
        }else{
            if(std::find(possiableMoves.begin(), possiableMoves.end(), squareIndex) != possiableMoves.end()){

                std::vector<Square> tempBoard = chessBoard;
                tempBoard[squareIndex] = tempBoard[selected_square];
                tempBoard[squareIndex].index = squareIndex;
                tempBoard[selected_square] = Square(selected_square);
                if(isSahInCheck(tempBoard,currentTurn)){
                    std::cout << "Gecersiz Hamle! bu hamle sahi tehdit altinda birakiyor" << std::endl;
                    return;
                }

                movePieceOnFunction(selected_square,squareIndex);
                checkPawnPromotion(chessBoard,squareIndex);
                updateVisualBoard();
                //printBoardInfo();
                std::cout << "Taş " << selected_square << " -> " << squareIndex << " hareket etti." << std::endl;
                selected_square = -1;
                possiableMoves.clear();
            }else{
                std::cout << "Geçersiz hamle!" << std::endl;
            }
        }
    }
}


int main(){

    firstChessInitalize();
    printBoardInfo();

    image = cv::imread("/home/halit/chess_project/object/chess_board.png");
    if(image.empty()){
        std::cout << "Goruntu Yuklenemedi !" << std::endl;
        return -1;
    }
    boardWithGrid = drawChessGrid(image);
    squareSize = boardWithGrid.rows / 8;
    cv::imshow("Chess", boardWithGrid);
    cv::setMouseCallback("Chess",onMouse,nullptr);

    firstChessBoard();


    while(true){

        cv::imshow("Chess",boardWithGrid);
        if(cv::waitKey(30) == 27) break; //esc break
    }
    
    return 0;
}






