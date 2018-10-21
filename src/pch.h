#ifndef PCH_H
#define PCH_H

#ifndef __cplusplus

#include <math.h>
#include <time.h>

#include <qglobal.h>

#else

#include <algorithm>
#include <cmath>
#include <ctime>
#include <functional>
#include <sstream>

#include <QtGlobal>

#include <Qt>
#include <QtMath>

#include <QAbstractAnimation>
#include <QAbstractButton>
#include <QAction>
#include <QAnimationGroup>
#include <QApplication>
#include <QAtomicPointer>
#include <QBitmap>
#include <QBoxLayout>
#include <QBrush>
#include <QBuffer>
#include <QButtonGroup>
#include <QCache>
#include <QCheckBox>
#include <QClipboard>
#include <QColor>
#include <QColorDialog>
#include <QComboBox>
#include <QCommandLinkButton>
#include <QCompleter>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QCursor>
#include <QDateTime>
#include <QDesktopServices>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QDrag>
#include <QEasingCurve>
#include <QElapsedTimer>
#include <QFile>
#include <QFileDialog>
#include <QFocusEvent>
#include <QFont>
#include <QFontDatabase>
#include <QFontDialog>
#include <QFontMetrics>
#include <QFormLayout>
#include <QFrame>
#include <QGlobalStatic>
#include <QGraphicsBlurEffect>
#include <QGraphicsColorizeEffect>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsEffect>
#include <QGraphicsItem>
#include <QGraphicsLinearLayout>
#include <QGraphicsObject>
#include <QGraphicsPixmapItem>
#include <QGraphicsProxyWidget>
#include <QGraphicsRectItem>
#include <QGraphicsRotation>
#include <QGraphicsScale>
#include <QGraphicsScene>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>
#include <QGraphicsTextItem>
#include <QGraphicsView>
#include <QGraphicsWidget>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHash>
#include <QHostAddress>
#include <QHostInfo>
#include <QIcon>
#include <QImage>
#include <QImageReader>
#include <QInputDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeyEvent>
#include <QLabel>
#include <QLayoutItem>
#include <QLineEdit>
#include <QList>
#include <QListWidget>
#include <QMainWindow>
#include <QMap>
#include <QMenu>
#include <QMessageBox>
#include <QMetaEnum>
#include <QMetaObject>
#include <QMultiHash>
#include <QMutex>
#include <QNetworkAccessManager>
#include <QNetworkInterface>
#include <QNetworkReply>
#include <QObject>
#include <QPaintEvent>
#include <QPainter>
#include <QPair>
#include <QPalette>
#include <QParallelAnimationGroup>
#include <QPauseAnimation>
#include <QPen>
#include <QPixmap>
#include <QPixmapCache>
#include <QPoint>
#include <QPointer>
#include <QProcess>
#include <QProgressBar>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QRadioButton>
#include <QRect>
#include <QRectF>
#include <QRegExp>
#include <QRegion>
#include <QScrollBar>
#include <QSemaphore>
#include <QSequentialAnimationGroup>
#include <QSet>
#include <QSettings>
#include <QSharedPointer>
#include <QShowEvent>
#include <QSignalMapper>
#include <QSize>
#include <QSpinBox>
#include <QSplitter>
#include <QStack>
#include <QStatusBar>
#include <QString>
#include <QStringList>
#include <QStyleOptionGraphicsItem>
#include <QSystemTrayIcon>
#include <QTabWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTextBrowser>
#include <QTextCodec>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextEdit>
#include <QTextItem>
#include <QTextOption>
#include <QTextStream>
#include <QThread>
#include <QTime>
#include <QTimer>
#include <QTimerEvent>
#include <QToolButton>
#include <QTransform>
#include <QUdpSocket>
#include <QVBoxLayout>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>
#include <QWaitCondition>
#include <QWidget>

#if QT_VERSION >= 0x050600
#include <QVersionNumber>
#endif

#ifdef Q_OS_WIN
#include <QWinTaskbarButton>
#include <QWinTaskbarProgress>
#endif

#endif

#endif
