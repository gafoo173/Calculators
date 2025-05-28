/*********************************************************************
 * Advanced Calculator Application in C++
 * ============================================
 * الوصف: تطبيق حاسبة متقدمة بواجهة رسومية باستخدام Qt. يدعم:
 *   - العمليات الحسابية الأساسية والعلمية (باستخدام محلل تعابير رياضية)
 *   - رسم الدوال الرياضية (تقريبياً بواسطة رسم نقاط)
 *   - حل المعادلات باستخدام طريقة الثنائيات (بسيطة)
 *   - العمليات التفاضلية والتكاملية (عددياً)
 *   - الحسابات الإحصائية
 *   - عمليات المصفوفات (جمع، طرح، ضرب، حساب المحدد والمعكوس لمصفوفات صغيرة)
 *   - تحويل الوحدات (الطول، الوزن، ودرجة الحرارة)
 *   - إعدادات (نموذجية)
 *
 * المطور: [اسمك]
 * تاريخ الإنشاء: 2025-05-28
 *********************************************************************/

#include <QApplication>
#include <QMainWindow>
#include <QTabWidget>
#include <QWidget>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QMessageBox>
#include <QTimer>
#include <QComboBox>
#include <QPainter>
#include <QScrollArea>
#include <QGroupBox>
#include <QDebug>

#include <cmath>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <string>
#include <regex>

// ---------------------------------------------------------------------
// جزء 1: محول تعابير رياضية: تحليل وتقييم تعبير رياضي باستخدام طريقة التنازل
// ---------------------------------------------------------------------
class ExpressionParser {
public:
    ExpressionParser(const std::string &s) : str(s), pos(0) {}

    double parse() {
        double result = parseExpression();
        skipWhitespace();
        if (pos != str.size())
            throw std::runtime_error("Unexpected characters at end of expression.");
        return result;
    }

private:
    std::string str;
    size_t pos;

    void skipWhitespace() {
        while (pos < str.size() && isspace(str[pos])) {
            pos++;
        }
    }
    double parseExpression() {
        double result = parseTerm();
        skipWhitespace();
        while (pos < str.size()) {
            char op = str[pos];
            if (op == '+' || op == '-') {
                pos++;
                double term = parseTerm();
                if (op == '+')
                    result += term;
                else
                    result -= term;
            } else {
                break;
            }
            skipWhitespace();
        }
        return result;
    }
    double parseTerm() {
        double result = parseFactor();
        skipWhitespace();
        while (pos < str.size()) {
            char op = str[pos];
            if (op == '*' || op == '/') {
                pos++;
                double factor = parseFactor();
                if (op == '*')
                    result *= factor;
                else
                    result /= factor;
            } else {
                break;
            }
            skipWhitespace();
        }
        return result;
    }
    double parseFactor() {
        double result = parseUnary();
        skipWhitespace();
        while (pos < str.size() && str[pos] == '^') {
            pos++;
            double exponent = parseUnary();
            result = pow(result, exponent);
            skipWhitespace();
        }
        return result;
    }
    double parseUnary() {
        skipWhitespace();
        if (pos < str.size() && (str[pos] == '+' || str[pos] == '-')) {
            char sign = str[pos];
            pos++;
            double factor = parseUnary();
            return (sign == '-') ? -factor : factor;
        } else {
            return parsePrimary();
        }
    }
    double parseNumber() {
        skipWhitespace();
        size_t start = pos;
        while (pos < str.size() && (isdigit(str[pos]) || str[pos] == '.'))
            pos++;
        double value = std::stod(str.substr(start, pos - start));
        return value;
    }
    double parsePrimary() {
        skipWhitespace();
        if (pos < str.size() && (isdigit(str[pos]) || str[pos] == '.')) {
            return parseNumber();
        }
        else if (pos < str.size() && isalpha(str[pos])) {
            std::string func;
            while (pos < str.size() &&
                   (isalpha(str[pos]) || str[pos] == '_'))
            {
                func.push_back(str[pos]);
                pos++;
            }
            skipWhitespace();
            if (pos < str.size() && str[pos] == '(') {
                pos++; // consume '('
                double arg = parseExpression();
                skipWhitespace();
                if (pos < str.size() && str[pos] == ')')
                    pos++;
                else
                    throw std::runtime_error("Expected ')'");
                if (func == "sin") return sin(arg);
                else if (func == "cos") return cos(arg);
                else if (func == "tan") return tan(arg);
                else if (func == "log") return log10(arg);
                else if (func == "ln") return log(arg);
                else if (func == "sqrt") return sqrt(arg);
                else if (func == "abs") return fabs(arg);
                else if (func == "asin") return asin(arg);
                else if (func == "acos") return acos(arg);
                else if (func == "atan") return atan(arg);
                else if (func == "exp") return exp(arg);
                else if (func == "floor") return floor(arg);
                else if (func == "ceil") return ceil(arg);
                else throw std::runtime_error("Unknown function: " + func);
            } else {
                // قد يكون ثابتا
                if (func == "pi") return M_PI;
                else if (func == "e") return M_E;
                else throw std::runtime_error("Unknown identifier: " + func);
            }
        }
        else if (pos < str.size() && str[pos] == '(') {
            pos++; // استهلاك (
            double result = parseExpression();
            skipWhitespace();
            if (pos < str.size() && str[pos] == ')') {
                pos++;
                return result;
            } else {
                throw std::runtime_error("Expected ')'");
            }
        }
        throw std::runtime_error("Unexpected character in expression.");
    }
};

double evaluateExpression(const std::string &expr) {
    ExpressionParser parser(expr);
    return parser.parse();
}

// ---------------------------------------------------------------------
// جزء 2: إدارة التاريخ والذاكرة (تخزين العمليات الحسابية والذاكرة)
// ---------------------------------------------------------------------
class HistoryManager {
public:
    void addEntry(const std::string &expr, double result) {
        history.push_back(std::make_pair(expr, result));
    }
    const std::vector<std::pair<std::string, double> >& getHistory() const {
        return history;
    }
    void clear() {
        history.clear();
    }
private:
    std::vector<std::pair<std::string, double> > history;
};

class MemoryManager {
public:
    MemoryManager() : memory(0.0) {}
    void add(double value) {
        memory += value;
    }
    void subtract(double value) {
        memory -= value;
    }
    double recall() const {
        return memory;
    }
    void clear() {
        memory = 0.0;
    }
private:
    double memory;
};

// ---------------------------------------------------------------------
// جزء 3: الحاسبة الأساسية (واجهة بسيطة للعمليات الحسابية)
// ---------------------------------------------------------------------
class BasicCalculatorWidget : public QWidget {
    Q_OBJECT
public:
    BasicCalculatorWidget(HistoryManager *histMgr, MemoryManager *memMgr, QWidget *parent = nullptr)
        : QWidget(parent), historyManager(histMgr), memoryManager(memMgr)
    {
        setupUI();
    }
private slots:
    void onButtonClicked() {
        QPushButton *btn = qobject_cast<QPushButton*>(sender());
        if (!btn) return;
        QString text = btn->text();
        if (text == "C") {
            inputEdit->clear();
            resultLabel->setText("");
        } else if (text == "=") {
            QString expr = inputEdit->text();
            try {
                std::string exprStr = expr.toStdString();
                // استبدال ^ بـ ** ليتوافق مع المحلل
                size_t pos = 0;
                while ((pos = exprStr.find("^", pos)) != std::string::npos) {
                    exprStr.replace(pos, 1, "**");
                    pos += 2;
                }
                double res = evaluateExpression(exprStr);
                resultLabel->setText(QString::number(res));
                historyManager->addEntry(expr.toStdString(), res);
            } catch (std::exception &e) {
                resultLabel->setText("خطأ: " + QString::fromStdString(e.what()));
            }
        } else if (text == "MC") {
            memoryManager->clear();
        } else if (text == "M+") {
            bool ok;
            double res = resultLabel->text().toDouble(&ok);
            if(ok)
                memoryManager->add(res);
        } else if (text == "M-") {
            bool ok;
            double res = resultLabel->text().toDouble(&ok);
            if(ok)
                memoryManager->subtract(res);
        } else if (text == "MR") {
            inputEdit->setText(inputEdit->text() + QString::number(memoryManager->recall()));
        } else if (text == "⌫") {
            QString current = inputEdit->text();
            inputEdit->setText(current.left(current.length() - 1));
        } else {
            inputEdit->setText(inputEdit->text() + text);
        }
    }
private:
    QLineEdit *inputEdit;
    QLabel *resultLabel;
    HistoryManager *historyManager;
    MemoryManager *memoryManager;
    void setupUI() {
        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        inputEdit = new QLineEdit(this);
        inputEdit->setReadOnly(false);
        inputEdit->setPlaceholderText("أدخل التعبير");
        inputEdit->setStyleSheet("font-size: 20px;");
        mainLayout->addWidget(inputEdit);
        
        resultLabel = new QLabel(this);
        resultLabel->setStyleSheet("background-color: lightgray; font-size: 20px; padding: 5px;");
        resultLabel->setMinimumHeight(40);
        resultLabel->setFrameShape(QFrame::Box);
        mainLayout->addWidget(resultLabel);

        // إنشاء شبكة للأزرار
        QGridLayout *grid = new QGridLayout();
        QStringList buttons = {
            "7", "8", "9", "/", "C", "⌫",
            "4", "5", "6", "*", "(", ")",
            "1", "2", "3", "-", "MR", "MC",
            "0", ".", "=", "+", "M+", "M-"
        };
        int row = 0, col = 0;
        for (const QString &btText : buttons) {
            QPushButton *btn = new QPushButton(btText, this);
            btn->setStyleSheet("font-size: 18px; padding: 10px;");
            btn->setMinimumSize(50, 50);
            connect(btn, &QPushButton::clicked, this, &BasicCalculatorWidget::onButtonClicked);
            grid->addWidget(btn, row, col);
            col++;
            if(col == 6) { col = 0; row++; }
        }
        mainLayout->addLayout(grid);
    }
};

// ---------------------------------------------------------------------
// جزء 4: الحاسبة العلمية (دعم الدوال العلمية والتعبيرات المعقدة)
// ---------------------------------------------------------------------
class ScientificCalculatorWidget : public QWidget {
    Q_OBJECT
public:
    ScientificCalculatorWidget(HistoryManager *histMgr, MemoryManager *memMgr, QWidget *parent = nullptr)
        : QWidget(parent), historyManager(histMgr), memoryManager(memMgr)
    {
        setupUI();
    }
private slots:
    void onSciButtonClicked() {
        QPushButton *btn = qobject_cast<QPushButton*>(sender());
        if(!btn) return;
        QString text = btn->text();
        if(text == "C") {
            exprEdit->clear();
            resLabel->setText("");
        } else if(text == "=") {
            QString expr = exprEdit->text();
            QString modExpr = expr;
            // تحويل ^ إلى ** (يمكن التعديل حسب الحاجة)
            modExpr.replace("^", "**");
            try {
                double res = evaluateExpression(modExpr.toStdString());
                resLabel->setText(QString::number(res));
                historyManager->addEntry(expr.toStdString(), res);
            } catch (std::exception &e) {
                resLabel->setText("خطأ: " + QString::fromStdString(e.what()));
            }
        } else if(text == "MC") {
            memoryManager->clear();
        } else if(text == "M+") {
            bool ok;
            double res = resLabel->text().toDouble(&ok);
            if(ok)
                memoryManager->add(res);
        } else if(text == "M-") {
            bool ok;
            double res = resLabel->text().toDouble(&ok);
            if(ok)
                memoryManager->subtract(res);
        } else if (text == "MR") {
            exprEdit->setText(exprEdit->text() + QString::number(memoryManager->recall()));
        } else if (text == "⌫") {
            QString current = exprEdit->text();
            exprEdit->setText(current.left(current.length() - 1));
        } else if (text == "x²") {
            exprEdit->setText(exprEdit->text() + "^2");
        } else if (text == "x³") {
            exprEdit->setText(exprEdit->text() + "^3");
        } else if (text == "1/x") {
            exprEdit->setText(exprEdit->text() + "1/");
        } else {
            exprEdit->setText(exprEdit->text() + text);
        }
    }
private:
    QLineEdit *exprEdit;
    QLabel *resLabel;
    HistoryManager *historyManager;
    MemoryManager *memoryManager;
    void setupUI() {
        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        exprEdit = new QLineEdit(this);
        exprEdit->setPlaceholderText("أدخل التعبير العلمي");
        exprEdit->setStyleSheet("font-size: 20px;");
        mainLayout->addWidget(exprEdit);

        resLabel = new QLabel(this);
        resLabel->setStyleSheet("background-color: lightgray; font-size: 20px; padding: 5px;");
        resLabel->setMinimumHeight(40);
        resLabel->setFrameShape(QFrame::Box);
        mainLayout->addWidget(resLabel);

        QGridLayout *grid = new QGridLayout();
        QStringList sciButtons = {
            "sin", "cos", "tan", "log", "ln", "C",
            "asin", "acos", "atan", "sqrt", "^", "⌫",
            "pi", "e", "(", ")", "=", "MR",
            "x²", "x³", "1/x", "exp", "abs", "MC",
            "!", "round", "floor", "ceil", "M+", "M-"
        };
        int row = 0, col = 0;
        for (const QString &btText : sciButtons) {
            QPushButton *btn = new QPushButton(btText, this);
            btn->setStyleSheet("font-size: 16px; padding: 8px;");
            btn->setMinimumSize(50, 50);
            connect(btn, &QPushButton::clicked, this, &ScientificCalculatorWidget::onSciButtonClicked);
            grid->addWidget(btn, row, col);
            col++;
            if(col == 6) { col = 0; row++; }
        }
        mainLayout->addLayout(grid);
    }
};

// ---------------------------------------------------------------------
// جزء 5: آلة الرسم البياني للدوال – رسم نقاط الدالة في نطاق محدد
// ---------------------------------------------------------------------
class GraphPlotWidget : public QWidget {
    Q_OBJECT
public:
    GraphPlotWidget(QWidget *parent = nullptr) : QWidget(parent) {
        functionStr = "";
        setMinimumSize(400, 300);
    }
    void setFunction(const QString &func) {
        functionStr = func;
        update(); // إعادة رسم
    }
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.fillRect(rect(), Qt::white);
        
        // رسم المحاور
        int w = width(), h = height();
        painter.setPen(Qt::black);
        painter.drawLine(0, h/2, w, h/2); // المحور الأفقي
        painter.drawLine(w/2, 0, w/2, h); // المحور العمودي

        // إذا لم يتم إدخال دالة، نخرج
        if(functionStr.isEmpty()) return;
        
        // جمع نقاط الدالة
        std::vector<QPointF> points;
        double xmin = -10.0, xmax = 10.0;
        int nPoints = w; // نقطة لكل بكسل تقريباً
        
        for (int i = 0; i < nPoints; i++) {
            double x = xmin + (xmax - xmin) * i / (nPoints - 1);
            double y = 0.0;
            try {
                // استبدال المتغير x في التعبير
                std::string exprStr = functionStr.toStdString();
                std::ostringstream oss;
                oss << x;
                std::string xstr = oss.str();
                
                // استبدال كل ظهور لـ 'x' بقيمته
                size_t pos = 0;
                while ((pos = exprStr.find('x', pos)) != std::string::npos) {
                    exprStr.replace(pos, 1, "(" + xstr + ")");
                    pos += xstr.length() + 2;
                }
                
                y = evaluateExpression(exprStr);
            } catch (...) {
                continue;
            }
            
            // تحويل الإحداثيات إلى النظام الرسومي
            double screenX = (x - xmin) * w / (xmax - xmin);
            double screenY = h/2 - y * (h/20.0); // مقياس بسيط
            
            // تجاهل القيم خارج النطاق
            if (screenY >= 0 && screenY < h) {
                points.push_back(QPointF(screenX, screenY));
            }
        }
        
        // رسم الخط البياني
        painter.setPen(QPen(Qt::blue, 2));
        for (int i = 1; i < (int)points.size(); i++) {
            painter.drawLine(points[i-1], points[i]);
        }
    }
private:
    QString functionStr;
};

class GraphingCalculatorWidget : public QWidget {
    Q_OBJECT
public:
    GraphingCalculatorWidget(QWidget *parent = nullptr) : QWidget(parent) {
        setupUI();
    }
private slots:
    void onPlotClicked() {
        QString func = functionEdit->text();
        graphWidget->setFunction(func);
    }
private:
    QLineEdit *functionEdit;
    QPushButton *plotButton;
    GraphPlotWidget *graphWidget;
    void setupUI() {
        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        QLabel *label = new QLabel("ادخل الدالة (باستخدام المتغير x):", this);
        label->setStyleSheet("font-size: 16px;");
        mainLayout->addWidget(label);
        
        functionEdit = new QLineEdit(this);
        functionEdit->setStyleSheet("font-size: 16px;");
        functionEdit->setText("sin(x)");
        mainLayout->addWidget(functionEdit);
        
        plotButton = new QPushButton("ارسم الدالة", this);
        plotButton->setStyleSheet("font-size: 16px;");
        connect(plotButton, &QPushButton::clicked, this, &GraphingCalculatorWidget::onPlotClicked);
        mainLayout->addWidget(plotButton);
        
        graphWidget = new GraphPlotWidget(this);
        graphWidget->setStyleSheet("background-color: white; border: 1px solid gray;");
        mainLayout->addWidget(graphWidget);
    }
};

// ---------------------------------------------------------------------
// جزء 6: حل المعادلات (باستخدام طريقة النصف لحل f(x)=0)
// ---------------------------------------------------------------------
class EquationSolverWidget : public QWidget {
    Q_OBJECT
public:
    EquationSolverWidget(QWidget *parent = nullptr) : QWidget(parent) {
        setupUI();
    }
private slots:
    void onSolveClicked() {
        // نستخدم طريقة النصف بطريقة مبسطة
        QString expr = equationEdit->text();
        double lower = lowerEdit->text().toDouble();
        double upper = upperEdit->text().toDouble();
        double tol = 1e-6;
        int maxIter = 100;
        double a = lower, b = upper;
        double fa, fb, fm;
        
        try {
            // تقييم f(a)
            std::string exprA = std::regex_replace(
                expr.toStdString(),
                std::regex("x"),
                "(" + std::to_string(a) + ")"
            );
            fa = evaluateExpression(exprA);
            
            // تقييم f(b)
            std::string exprB = std::regex_replace(
                expr.toStdString(),
                std::regex("x"),
                "(" + std::to_string(b) + ")"
            );
            fb = evaluateExpression(exprB);
        } catch (...) {
            resultEdit->setPlainText("خطأ في تقييم f(a) أو f(b).");
            return;
        }
        
        if(fa * fb > 0) {
            resultEdit->setPlainText("لا يوجد تغيير في الإشارة، لا يمكن تطبيق طريقة النصف.");
            return;
        }
        
        double m = a;
        for (int i = 0; i < maxIter; i++) {
            m = (a + b) / 2.0;
            try {
                // تقييم f(m)
                std::string exprM = std::regex_replace(
                    expr.toStdString(),
                    std::regex("x"),
                    "(" + std::to_string(m) + ")"
                );
                fm = evaluateExpression(exprM);
            } catch (...) {
                resultEdit->setPlainText("خطأ في تقييم f(m).");
                return;
            }
            
            if (fabs(fm) < tol)
                break;
                
            if(fa * fm < 0) {
                b = m; 
                fb = fm;
            } else {
                a = m; 
                fa = fm;
            }
        }
        resultEdit->setPlainText("الجذر التقريبي: " + QString::number(m) + 
                               "\nعدد التكرارات: " + QString::number(maxIter));
    }
private:
    QLineEdit *equationEdit;
    QLineEdit *lowerEdit;
    QLineEdit *upperEdit;
    QPushButton *solveButton;
    QTextEdit *resultEdit;
    void setupUI() {
        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        QLabel *inst = new QLabel("ادخل المعادلة بصيغة f(x)=0، وحدد الحدود [a,b]:", this);
        inst->setStyleSheet("font-size: 16px;");
        mainLayout->addWidget(inst);
        
        equationEdit = new QLineEdit(this);
        equationEdit->setPlaceholderText("أدخل تعبير f(x)");
        equationEdit->setStyleSheet("font-size: 16px;");
        equationEdit->setText("x^2 - 4");
        mainLayout->addWidget(equationEdit);
        
        QHBoxLayout *rangeLayout = new QHBoxLayout();
        lowerEdit = new QLineEdit(this);
        lowerEdit->setPlaceholderText("a");
        lowerEdit->setStyleSheet("font-size: 16px;");
        lowerEdit->setText("0");
        upperEdit = new QLineEdit(this);
        upperEdit->setPlaceholderText("b");
        upperEdit->setStyleSheet("font-size: 16px;");
        upperEdit->setText("3");
        rangeLayout->addWidget(lowerEdit);
        rangeLayout->addWidget(upperEdit);
        mainLayout->addLayout(rangeLayout);
        
        solveButton = new QPushButton("حل المعادلة", this);
        solveButton->setStyleSheet("font-size: 16px;");
        connect(solveButton, &QPushButton::clicked, this, &EquationSolverWidget::onSolveClicked);
        mainLayout->addWidget(solveButton);
        
        resultEdit = new QTextEdit(this);
        resultEdit->setReadOnly(true);
        resultEdit->setStyleSheet("font-size: 16px;");
        mainLayout->addWidget(resultEdit);
    }
};

// ---------------------------------------------------------------------
// جزء 7: العمليات التفاضلية والتكاملية العددية
// ---------------------------------------------------------------------
class CalculusWidget : public QWidget {
    Q_OBJECT
public:
    CalculusWidget(QWidget *parent = nullptr) : QWidget(parent) {
        setupUI();
    }
private slots:
    void onDifferentiateClicked() {
        QString expr = calcEdit->text();
        double x = pointEdit->text().toDouble();
        double h = 1e-5;
        double f_plus, f_minus;
        try {
            // تقييم f(x+h) و f(x-h)
            std::string exprPlus = std::regex_replace(
                expr.toStdString(),
                std::regex("x"),
                "(" + std::to_string(x+h) + ")"
            );
            f_plus = evaluateExpression(exprPlus);
            
            std::string exprMinus = std::regex_replace(
                expr.toStdString(),
                std::regex("x"),
                "(" + std::to_string(x-h) + ")"
            );
            f_minus = evaluateExpression(exprMinus);
        } catch (...) {
            calcResult->append("خطأ في حساب المشتقة.");
            return;
        }
        double derivative = (f_plus - f_minus) / (2 * h);
        calcResult->append("مشتقة f عند x = " + QString::number(x) + " تساوي: " + QString::number(derivative));
    }
    
    void onIntegrateClicked() {
        QString expr = calcEdit->text();
        double a = lowerIntEdit->text().toDouble();
        double b = upperIntEdit->text().toDouble();
        int N = 1000;
        double h = (b - a) / N;
        double integral = 0.0;
        
        try {
            std::string s_expr = expr.toStdString();
            for (int i = 0; i <= N; i++) {
                double x = a + i * h;
                std::string exprX = std::regex_replace(
                    s_expr,
                    std::regex("x"),
                    "(" + std::to_string(x) + ")"
                );
                double fx = evaluateExpression(exprX);
                
                if (i == 0 || i == N)
                    integral += fx;
                else if (i % 2 == 0)
                    integral += 2 * fx;
                else
                    integral += 4 * fx;
            }
            integral = (integral * h) / 3;
        } catch (...) {
            calcResult->append("خطأ في حساب التكامل.");
            return;
        }
        calcResult->append("التكامل من " + QString::number(a) + " إلى " + QString::number(b) + " يساوي: " + QString::number(integral));
    }
    
    void onLimitClicked() {
        QString expr = calcEdit->text();
        double x0 = limitEdit->text().toDouble();
        double h = 1e-5;
        double f1, f2;
        try {
            std::string expr1 = std::regex_replace(
                expr.toStdString(),
                std::regex("x"),
                "(" + std::to_string(x0+h) + ")"
            );
            f1 = evaluateExpression(expr1);
            
            std::string expr2 = std::regex_replace(
                expr.toStdString(),
                std::regex("x"),
                "(" + std::to_string(x0-h) + ")"
            );
            f2 = evaluateExpression(expr2);
        } catch (...) {
            calcResult->append("خطأ في حساب النهاية.");
            return;
        }
        double lim_val = (f1 + f2) / 2;
        calcResult->append("نهاية f عند x = " + QString::number(x0) + " تقريباً: " + QString::number(lim_val));
    }
private:
    QLineEdit *calcEdit;
    QLineEdit *pointEdit;
    QLineEdit *lowerIntEdit;
    QLineEdit *upperIntEdit;
    QLineEdit *limitEdit;
    QTextEdit *calcResult;
    QPushButton *diffButton, *intButton, *limitButton;
    void setupUI() {
        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        QLabel *instr = new QLabel("أدخل التعبير الرياضي (باستخدام x):", this);
        instr->setStyleSheet("font-size: 16px;");
        mainLayout->addWidget(instr);
        
        calcEdit = new QLineEdit(this);
        calcEdit->setStyleSheet("font-size: 16px;");
        calcEdit->setText("sin(x)");
        mainLayout->addWidget(calcEdit);
        
        QHBoxLayout *row1 = new QHBoxLayout();
        QLabel *pointLabel = new QLabel("لحساب المشتقة عند x =", this);
        pointLabel->setStyleSheet("font-size: 16px;");
        pointEdit = new QLineEdit(this);
        pointEdit->setStyleSheet("font-size: 16px;");
        pointEdit->setText("0");
        diffButton = new QPushButton("احسب المشتقة", this);
        diffButton->setStyleSheet("font-size: 16px;");
        connect(diffButton, &QPushButton::clicked, this, &CalculusWidget::onDifferentiateClicked);
        row1->addWidget(pointLabel);
        row1->addWidget(pointEdit);
        row1->addWidget(diffButton);
        mainLayout->addLayout(row1);
        
        QHBoxLayout *row2 = new QHBoxLayout();
        QLabel *intLabel = new QLabel("تكامل من", this);
        intLabel->setStyleSheet("font-size: 16px;");
        lowerIntEdit = new QLineEdit(this);
        lowerIntEdit->setStyleSheet("font-size: 16px;");
        lowerIntEdit->setText("0");
        QLabel *toLabel = new QLabel("إلى", this);
        toLabel->setStyleSheet("font-size: 16px;");
        upperIntEdit = new QLineEdit(this);
        upperIntEdit->setStyleSheet("font-size: 16px;");
        upperIntEdit->setText("3.14159");
        intButton = new QPushButton("احسب التكامل", this);
        intButton->setStyleSheet("font-size: 16px;");
        connect(intButton, &QPushButton::clicked, this, &CalculusWidget::onIntegrateClicked);
        row2->addWidget(intLabel);
        row2->addWidget(lowerIntEdit);
        row2->addWidget(toLabel);
        row2->addWidget(upperIntEdit);
        row2->addWidget(intButton);
        mainLayout->addLayout(row2);
        
        QHBoxLayout *row3 = new QHBoxLayout();
        QLabel *limitLabel = new QLabel("احسب النهاية عند x =", this);
        limitLabel->setStyleSheet("font-size: 16px;");
        limitEdit = new QLineEdit(this);
        limitEdit->setStyleSheet("font-size: 16px;");
        limitEdit->setText("0");
        limitButton = new QPushButton("احسب النهاية", this);
        limitButton->setStyleSheet("font-size: 16px;");
        connect(limitButton, &QPushButton::clicked, this, &CalculusWidget::onLimitClicked);
        row3->addWidget(limitLabel);
        row3->addWidget(limitEdit);
        row3->addWidget(limitButton);
        mainLayout->addLayout(row3);
        
        calcResult = new QTextEdit(this);
        calcResult->setReadOnly(true);
        calcResult->setStyleSheet("font-size: 16px;");
        mainLayout->addWidget(calcResult);
    }
};

// ---------------------------------------------------------------------
// جزء 8: الحسابات الإحصائية
// ---------------------------------------------------------------------
class StatisticsWidget : public QWidget {
    Q_OBJECT
public:
    StatisticsWidget(QWidget *parent = nullptr) : QWidget(parent) {
        setupUI();
    }
private slots:
    void onCalculateStatsClicked() {
        QString numsStr = numbersEdit->text();
        QStringList parts = numsStr.split(",", Qt::SkipEmptyParts);
        std::vector<double> values;
        for (const QString &part : parts) {
            bool ok;
            double val = part.trimmed().toDouble(&ok);
            if(ok)
                values.push_back(val);
        }
        if(values.empty()) {
            statsResult->setPlainText("لا توجد أرقام صالحة.");
            return;
        }
        // حساب المتوسط
        double sum = 0;
        for (double v : values)
            sum += v;
        double mean = sum / values.size();
        // حساب الوسيط
        std::sort(values.begin(), values.end());
        double median = (values.size()%2==0) ?
            (values[values.size()/2 - 1] + values[values.size()/2]) / 2.0 :
            values[values.size()/2];
        // حساب التباين والانحراف المعياري
        double var = 0;
        for(double v : values)
            var += (v - mean) * (v - mean);
        if(values.size() > 1)
            var /= (values.size()-1);
        double stdev = sqrt(var);
        
        // حساب القيم القصوى والدنيا
        double minVal = *std::min_element(values.begin(), values.end());
        double maxVal = *std::max_element(values.begin(), values.end());
        
        QString resultText;
        resultText += "عدد القيم: " + QString::number(values.size()) + "\n";
        resultText += "المتوسط الحسابي: " + QString::number(mean) + "\n";
        resultText += "الوسيط: " + QString::number(median) + "\n";
        resultText += "التباين: " + QString::number(var) + "\n";
        resultText += "الانحراف المعياري: " + QString::number(stdev) + "\n";
        resultText += "الحد الأدنى: " + QString::number(minVal) + "\n";
        resultText += "الحد الأقصى: " + QString::number(maxVal) + "\n";
        statsResult->setPlainText(resultText);
    }
private:
    QLineEdit *numbersEdit;
    QTextEdit *statsResult;
    QPushButton *calcButton;
    void setupUI() {
        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        QLabel *instr = new QLabel("أدخل الأرقام مفصولة بفواصل:", this);
        instr->setStyleSheet("font-size: 16px;");
        mainLayout->addWidget(instr);
        
        numbersEdit = new QLineEdit(this);
        numbersEdit->setStyleSheet("font-size: 16px;");
        numbersEdit->setText("1, 2, 3, 4, 5");
        mainLayout->addWidget(numbersEdit);
        
        calcButton = new QPushButton("احسب الإحصائيات", this);
        calcButton->setStyleSheet("font-size: 16px;");
        connect(calcButton, &QPushButton::clicked, this, &StatisticsWidget::onCalculateStatsClicked);
        mainLayout->addWidget(calcButton);
        
        statsResult = new QTextEdit(this);
        statsResult->setReadOnly(true);
        statsResult->setStyleSheet("font-size: 16px;");
        mainLayout->addWidget(statsResult);
    }
};

// ---------------------------------------------------------------------
// جزء 9: عمليات المصفوفات (جمع، طرح، ضرب، حساب المحدد والمعكوس)
// ---------------------------------------------------------------------
class MatrixCalculatorWidget : public QWidget {
    Q_OBJECT
public:
    MatrixCalculatorWidget(QWidget *parent = nullptr) : QWidget(parent) {
        setupUI();
    }
private slots:
    void onComputeClicked() {
        QString op = opCombo->currentText();
        QString matrixAText = matrixAEdit->toPlainText();
        // تحويل نص المصفوفة إلى مصفوفة من الأرقام (vector<vector<double>>)
        std::vector<std::vector<double>> A;
        if(!parseMatrix(matrixAText, A)) {
            resultEdit->setPlainText("خطأ في قراءة المصفوفة A.");
            return;
        }
        std::vector<std::vector<double>> B;
        if(op == "جمع" || op == "طرح" || op == "ضرب") {
            QString matrixBText = matrixBEdit->toPlainText();
            if(!parseMatrix(matrixBText, B)) {
                resultEdit->setPlainText("خطأ في قراءة المصفوفة B.");
                return;
            }
        }
        std::vector<std::vector<double>> res;
        try {
            if(op == "جمع") {
                res = addMatrix(A, B);
                resultEdit->setPlainText(matrixToString(res));
            } else if(op == "طرح") {
                res = subtractMatrix(A, B);
                resultEdit->setPlainText(matrixToString(res));
            } else if(op == "ضرب") {
                res = multiplyMatrix(A, B);
                resultEdit->setPlainText(matrixToString(res));
            } else if(op == "محدد") {
                if (A.size() != A[0].size()) {
                    resultEdit->setPlainText("يجب أن تكون المصفوفة مربعة.");
                    return;
                }
                double det = determinant(A);
                resultEdit->setPlainText("المحدد: " + QString::number(det));
            } else if(op == "معكوس") {
                if (A.size() != A[0].size()) {
                    resultEdit->setPlainText("يجب أن تكون المصفوفة مربعة.");
                    return;
                }
                res = inverseMatrix(A);
                resultEdit->setPlainText(matrixToString(res));
            }
        } catch (std::exception &ex) {
            resultEdit->setPlainText("حدث خطأ أثناء عملية المصفوفة: " + QString(ex.what()));
        }
    }
    
    void onGenerateClicked() {
        int rows = rowSpin->value();
        int cols = colSpin->value();
        matrixAEdit->clear();
        matrixBEdit->clear();
        // توليد مصفوفة عشوائية للمصفوفة A
        QString matrixAText;
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                matrixAText += QString::number(rand() % 10);
                if (j < cols - 1) matrixAText += ", ";
            }
            if (i < rows - 1) matrixAText += "\n";
        }
        matrixAEdit->setPlainText(matrixAText);
        
        // توليد مصفوفة عشوائية للمصفوفة B (إذا لزم الأمر)
        if (opCombo->currentText() == "جمع" || 
            opCombo->currentText() == "طرح" || 
            opCombo->currentText() == "ضرب") {
            int rowsB = (opCombo->currentText() == "ضرب") ? cols : rows;
            int colsB = (opCombo->currentText() == "ضرب") ? colSpin->value() : cols;
            QString matrixBText;
            for (int i = 0; i < rowsB; i++) {
                for (int j = 0; j < colsB; j++) {
                    matrixBText += QString::number(rand() % 10);
                    if (j < colsB - 1) matrixBText += ", ";
                }
                if (i < rowsB - 1) matrixBText += "\n";
            }
            matrixBEdit->setPlainText(matrixBText);
        }
    }
private:
    QTextEdit *matrixAEdit;
    QTextEdit *matrixBEdit;
    QComboBox *opCombo;
    QTextEdit *resultEdit;
    QSpinBox *rowSpin;
    QSpinBox *colSpin;
    
    bool parseMatrix(const QString &text, std::vector<std::vector<double>> &mat) {
        mat.clear();
        QStringList rows = text.split("\n", Qt::SkipEmptyParts);
        for(const QString &row : rows) {
            QStringList numbers = row.split(QRegExp("[, \\t]+"), Qt::SkipEmptyParts);
            std::vector<double> rowVals;
            for (const QString &numStr : numbers) {
                bool ok;
                double num = numStr.toDouble(&ok);
                if(!ok)
                    return false;
                rowVals.push_back(num);
            }
            mat.push_back(rowVals);
        }
        return true;
    }
    
    std::vector<std::vector<double>> addMatrix(const std::vector<std::vector<double>> &A,
                                                 const std::vector<std::vector<double>> &B) {
        if(A.size() != B.size() || A[0].size() != B[0].size())
            throw std::runtime_error("أبعاد المصفوفات غير متطابقة.");
        std::vector<std::vector<double>> C = A;
        for (size_t i = 0; i < A.size(); i++)
            for (size_t j = 0; j < A[0].size(); j++)
                C[i][j] += B[i][j];
        return C;
    }
    
    std::vector<std::vector<double>> subtractMatrix(const std::vector<std::vector<double>> &A,
                                                      const std::vector<std::vector<double>> &B) {
        if(A.size() != B.size() || A[0].size() != B[0].size())
            throw std::runtime_error("أبعاد المصفوفات غير متطابقة.");
        std::vector<std::vector<double>> C = A;
        for (size_t i = 0; i < A.size(); i++)
            for (size_t j = 0; j < A[0].size(); j++)
                C[i][j] -= B[i][j];
        return C;
    }
    
    std::vector<std::vector<double>> multiplyMatrix(const std::vector<std::vector<double>> &A,
                                                      const std::vector<std::vector<double>> &B) {
        if(A[0].size() != B.size())
            throw std::runtime_error("أبعاد المصفوفات غير متوافقة للضرب.");
        size_t m = A.size(), n = B[0].size(), p = A[0].size();
        std::vector<std::vector<double>> C(m, std::vector<double>(n, 0));
        for (size_t i = 0; i < m; i++)
            for (size_t j = 0; j < n; j++)
                for (size_t k = 0; k < p; k++)
                    C[i][j] += A[i][k] * B[k][j];
        return C;
    }
    
    double determinant(const std::vector<std::vector<double>> &M) {
        size_t n = M.size();
        if (n == 1) return M[0][0];
        if (n == 2) return M[0][0]*M[1][1] - M[0][1]*M[1][0];
        if (n == 3) {
            return M[0][0]*(M[1][1]*M[2][2] - M[1][2]*M[2][1]) -
                   M[0][1]*(M[1][0]*M[2][2] - M[1][2]*M[2][0]) +
                   M[0][2]*(M[1][0]*M[2][1] - M[1][1]*M[2][0]);
        }
        throw std::runtime_error("حساب المحدد غير مدعوم للمصفوفات أكبر من 3x3.");
    }
    
    std::vector<std::vector<double>> inverseMatrix(const std::vector<std::vector<double>> &M) {
        size_t n = M.size();
        if (n == 1) {
            return {{1.0 / M[0][0]}};
        } else if (n == 2) {
            double det = determinant(M);
            if (fabs(det) < 1e-10) 
                throw std::runtime_error("المصفوفة ليس لها معكوس (المحدد صفر).");
            return {
                { M[1][1]/det, -M[0][1]/det },
                { -M[1][0]/det, M[0][0]/det }
            };
        } else if (n == 3) {
            double det = determinant(M);
            if (fabs(det) < 1e-10) 
                throw std::runtime_error("المصفوفة ليس لها معكوس (المحدد صفر).");
            
            std::vector<std::vector<double>> adj(3, std::vector<double>(3));
            // حساب مصفوفة الإضافة (adjugate)
            adj[0][0] = M[1][1]*M[2][2] - M[1][2]*M[2][1];
            adj[0][1] = -(M[1][0]*M[2][2] - M[1][2]*M[2][0]);
            adj[0][2] = M[1][0]*M[2][1] - M[1][1]*M[2][0];
            
            adj[1][0] = -(M[0][1]*M[2][2] - M[0][2]*M[2][1]);
            adj[1][1] = M[0][0]*M[2][2] - M[0][2]*M[2][0];
            adj[1][2] = -(M[0][0]*M[2][1] - M[0][1]*M[2][0]);
            
            adj[2][0] = M[0][1]*M[1][2] - M[0][2]*M[1][1];
            adj[2][1] = -(M[0][0]*M[1][2] - M[0][2]*M[1][0]);
            adj[2][2] = M[0][0]*M[1][1] - M[0][1]*M[1][0];
            
            // نقل المصفوفة (لأن الإضافة هي نقل مصفوفة العوامل)
            for (size_t i = 0; i < n; i++)
                for (size_t j = 0; j < i; j++)
                    std::swap(adj[i][j], adj[j][i]);
            
            // القسمة على المحدد
            for (size_t i = 0; i < n; i++)
                for (size_t j = 0; j < n; j++)
                    adj[i][j] /= det;
                    
            return adj;
        }
        throw std::runtime_error("حساب المعكوس غير مدعوم للمصفوفات أكبر من 3x3.");
    }
    
    QString matrixToString(const std::vector<std::vector<double>> &mat) {
        QString res;
        for (size_t i = 0; i < mat.size(); i++) {
            for (size_t j = 0; j < mat[i].size(); j++) {
                res += QString::number(mat[i][j], 'f', 4);
                if (j < mat[i].size() - 1)
                    res += ", ";
            }
            res += "\n";
        }
        return res;
    }
    
    void setupUI() {
        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        
        QHBoxLayout *controlLayout = new QHBoxLayout();
        QLabel *opLabel = new QLabel("العملية:", this);
        opCombo = new QComboBox(this);
        opCombo->addItems({"جمع", "طرح", "ضرب", "محدد", "معكوس"});
        controlLayout->addWidget(opLabel);
        controlLayout->addWidget(opCombo);
        
        QLabel *rowLabel = new QLabel("الصفوف:", this);
        rowSpin = new QSpinBox(this);
        rowSpin->setRange(1, 10);
        rowSpin->setValue(2);
        controlLayout->addWidget(rowLabel);
        controlLayout->addWidget(rowSpin);
        
        QLabel *colLabel = new QLabel("الأعمدة:", this);
        colSpin = new QSpinBox(this);
        colSpin->setRange(1, 10);
        colSpin->setValue(2);
        controlLayout->addWidget(colLabel);
        controlLayout->addWidget(colSpin);
        
        QPushButton *generateBtn = new QPushButton("توليد", this);
        connect(generateBtn, &QPushButton::clicked, this, &MatrixCalculatorWidget::onGenerateClicked);
        controlLayout->addWidget(generateBtn);
        
        QPushButton *computeBtn = new QPushButton("احسب", this);
        connect(computeBtn, &QPushButton::clicked, this, &MatrixCalculatorWidget::onComputeClicked);
        controlLayout->addWidget(computeBtn);
        
        mainLayout->addLayout(controlLayout);
        
        QHBoxLayout *matrixLayout = new QHBoxLayout();
        QGroupBox *groupA = new QGroupBox("المصفوفة A", this);
        QVBoxLayout *groupALayout = new QVBoxLayout();
        matrixAEdit = new QTextEdit(this);
        matrixAEdit->setMinimumSize(200, 150);
        groupALayout->addWidget(matrixAEdit);
        groupA->setLayout(groupALayout);
        matrixLayout->addWidget(groupA);
        
        QGroupBox *groupB = new QGroupBox("المصفوفة B", this);
        QVBoxLayout *groupBLayout = new QVBoxLayout();
        matrixBEdit = new QTextEdit(this);
        matrixBEdit->setMinimumSize(200, 150);
        groupBLayout->addWidget(matrixBEdit);
        groupB->setLayout(groupBLayout);
        matrixLayout->addWidget(groupB);
        
        mainLayout->addLayout(matrixLayout);
        
        QGroupBox *resultGroup = new QGroupBox("النتيجة", this);
        QVBoxLayout *resultLayout = new QVBoxLayout();
        resultEdit = new QTextEdit(this);
        resultEdit->setReadOnly(true);
        resultEdit->setMinimumHeight(150);
        resultLayout->addWidget(resultEdit);
        resultGroup->setLayout(resultLayout);
        mainLayout->addWidget(resultGroup);
    }
};

// ---------------------------------------------------------------------
// جزء 10: تحويل الوحدات
// ---------------------------------------------------------------------
class UnitConverterWidget : public QWidget {
    Q_OBJECT
public:
    UnitConverterWidget(QWidget *parent = nullptr) : QWidget(parent) {
        setupUI();
    }
private slots:
    void onConvertClicked() {
        double value = inputEdit->text().toDouble();
        QString fromUnit = fromCombo->currentText();
        QString toUnit = toCombo->currentText();
        double result = 0.0;
        
        if (categoryCombo->currentText() == "الطول") {
            // تحويل الطول: متر، كيلومتر، قدم، بوصة، ميل
            if (fromUnit == "متر" && toUnit == "كيلومتر") result = value / 1000;
            else if (fromUnit == "متر" && toUnit == "قدم") result = value * 3.28084;
            else if (fromUnit == "متر" && toUnit == "بوصة") result = value * 39.3701;
            else if (fromUnit == "متر" && toUnit == "ميل") result = value / 1609.34;
            else if (fromUnit == "كيلومتر" && toUnit == "متر") result = value * 1000;
            else if (fromUnit == "قدم" && toUnit == "متر") result = value / 3.28084;
            else if (fromUnit == "بوصة" && toUnit == "متر") result = value / 39.3701;
            else if (fromUnit == "ميل" && toUnit == "متر") result = value * 1609.34;
            else result = value; // نفس الوحدة
        }
        else if (categoryCombo->currentText() == "الوزن") {
            // تحويل الوزن: كيلوجرام، جرام، رطل، أوقية
            if (fromUnit == "كيلوجرام" && toUnit == "جرام") result = value * 1000;
            else if (fromUnit == "كيلوجرام" && toUnit == "رطل") result = value * 2.20462;
            else if (fromUnit == "كيلوجرام" && toUnit == "أوقية") result = value * 35.274;
            else if (fromUnit == "جرام" && toUnit == "كيلوجرام") result = value / 1000;
            else if (fromUnit == "رطل" && toUnit == "كيلوجرام") result = value / 2.20462;
            else if (fromUnit == "أوقية" && toUnit == "كيلوجرام") result = value / 35.274;
            else result = value;
        }
        else if (categoryCombo->currentText() == "درجة الحرارة") {
            // تحويل الحرارة: مئوية، فهرنهايت، كلفن
            if (fromUnit == "مئوية" && toUnit == "فهرنهايت") result = (value * 9/5) + 32;
            else if (fromUnit == "مئوية" && toUnit == "كلفن") result = value + 273.15;
            else if (fromUnit == "فهرنهايت" && toUnit == "مئوية") result = (value - 32) * 5/9;
            else if (fromUnit == "فهرنهايت" && toUnit == "كلفن") result = (value - 32) * 5/9 + 273.15;
            else if (fromUnit == "كلفن" && toUnit == "مئوية") result = value - 273.15;
            else if (fromUnit == "كلفن" && toUnit == "فهرنهايت") result = (value - 273.15) * 9/5 + 32;
            else result = value;
        }
        resultEdit->setText(QString::number(result));
    }
    
    void onCategoryChanged() {
        QString category = categoryCombo->currentText();
        fromCombo->clear();
        toCombo->clear();
        
        if (category == "الطول") {
            fromCombo->addItems({"متر", "كيلومتر", "قدم", "بوصة", "ميل"});
            toCombo->addItems({"متر", "كيلومتر", "قدم", "بوصة", "ميل"});
        } else if (category == "الوزن") {
            fromCombo->addItems({"كيلوجرام", "جرام", "رطل", "أوقية"});
            toCombo->addItems({"كيلوجرام", "جرام", "رطل", "أوقية"});
        } else if (category == "درجة الحرارة") {
            fromCombo->addItems({"مئوية", "فهرنهايت", "كلفن"});
            toCombo->addItems({"مئوية", "فهرنهايت", "كلفن"});
        }
    }
private:
    QComboBox *categoryCombo;
    QComboBox *fromCombo;
    QComboBox *toCombo;
    QLineEdit *inputEdit;
    QLabel *resultEdit;
    
    void setupUI() {
        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        
        QLabel *title = new QLabel("تحويل الوحدات", this);
        title->setStyleSheet("font-size: 18px; font-weight: bold;");
        mainLayout->addWidget(title);
        
        QHBoxLayout *categoryLayout = new QHBoxLayout();
        QLabel *catLabel = new QLabel("الفئة:", this);
        categoryCombo = new QComboBox(this);
        categoryCombo->addItems({"الطول", "الوزن", "درجة الحرارة"});
        connect(categoryCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onCategoryChanged()));
        categoryLayout->addWidget(catLabel);
        categoryLayout->addWidget(categoryCombo);
        mainLayout->addLayout(categoryLayout);
        
        QHBoxLayout *conversionLayout = new QHBoxLayout();
        inputEdit = new QLineEdit(this);
        inputEdit->setPlaceholderText("القيمة");
        fromCombo = new QComboBox(this);
        toCombo = new QComboBox(this);
        QPushButton *convertBtn = new QPushButton("تحويل", this);
        connect(convertBtn, &QPushButton::clicked, this, &UnitConverterWidget::onConvertClicked);
        
        conversionLayout->addWidget(inputEdit);
        conversionLayout->addWidget(fromCombo);
        conversionLayout->addWidget(new QLabel("إلى", this));
        conversionLayout->addWidget(toCombo);
        conversionLayout->addWidget(convertBtn);
        mainLayout->addLayout(conversionLayout);
        
        QLabel *resultLabel = new QLabel("النتيجة:", this);
        resultEdit = new QLabel(this);
        resultEdit->setStyleSheet("font-size: 16px; background-color: lightgray; padding: 5px;");
        resultEdit->setMinimumHeight(30);
        
        mainLayout->addWidget(resultLabel);
        mainLayout->addWidget(resultEdit);
        
        // تهيئة الوحدات
        onCategoryChanged();
    }
};

// ---------------------------------------------------------------------
// جزء 11: النافذة الرئيسية
// ---------------------------------------------------------------------
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow() : QMainWindow() {
        setWindowTitle("الحاسبة المتقدمة");
        setMinimumSize(800, 600);
        
        historyManager = new HistoryManager();
        memoryManager = new MemoryManager();
        
        QTabWidget *tabWidget = new QTabWidget(this);
        
        // إضافة التبويبات
        tabWidget->addTab(new BasicCalculatorWidget(historyManager, memoryManager), "حاسبة أساسية");
        tabWidget->addTab(new ScientificCalculatorWidget(historyManager, memoryManager), "حاسبة علمية");
        tabWidget->addTab(new GraphingCalculatorWidget(), "رسم بياني");
        tabWidget->addTab(new EquationSolverWidget(), "حل المعادلات");
        tabWidget->addTab(new CalculusWidget(), "تفاضل وتكامل");
        tabWidget->addTab(new StatisticsWidget(), "إحصائيات");
        tabWidget->addTab(new MatrixCalculatorWidget(), "مصفوفات");
        tabWidget->addTab(new UnitConverterWidget(), "تحويل الوحدات");
        
        setCentralWidget(tabWidget);
    }
    
    ~MainWindow() {
        delete historyManager;
        delete memoryManager;
    }
    
private:
    HistoryManager *historyManager;
    MemoryManager *memoryManager;
};

// ---------------------------------------------------------------------
// تشغيل التطبيق
// ---------------------------------------------------------------------
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow mainWin;
    mainWin.show();
    return app.exec();
}

#include "main.moc"
