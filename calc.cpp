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
                else if (func == "log") return log(arg);
                else if (func == "sqrt") return sqrt(arg);
                else if (func == "abs") return fabs(arg);
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
                double res = evaluateExpression(expr.toStdString());
                resultLabel->setText(QString::number(res));
                historyManager->addEntry(expr.toStdString(), res);
            } catch (std::exception &e) {
                resultLabel->setText("خطأ");
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
        resultLabel->setStyleSheet("background-color: lightgray; font-size: 20px;");
        resultLabel->setMinimumHeight(40);
        mainLayout->addWidget(resultLabel);

        // إنشاء شبكة للأزرار
        QGridLayout *grid = new QGridLayout();
        QStringList buttons = {"7", "8", "9", "/",
                               "4", "5", "6", "*",
                               "1", "2", "3", "-",
                               "0", ".", "=", "+",
                               "C", "MC", "M+", "M-"};
        int row = 0, col = 0;
        for (const QString &btText : buttons) {
            QPushButton *btn = new QPushButton(btText, this);
            btn->setStyleSheet("font-size: 18px;");
            connect(btn, &QPushButton::clicked, this, &BasicCalculatorWidget::onButtonClicked);
            grid->addWidget(btn, row, col);
            col++;
            if(col == 4) { col = 0; row++; }
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
                resLabel->setText("خطأ");
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
        resLabel->setStyleSheet("background-color: lightgray; font-size: 20px;");
        resLabel->setMinimumHeight(40);
        mainLayout->addWidget(resLabel);

        QGridLayout *grid = new QGridLayout();
        QStringList sciButtons = {"sin", "cos", "tan", "log", "ln",
                                  "asin", "acos", "atan", "sqrt", "^",
                                  "(", ")", "pi", "e", "factorial",
                                  "exp", "abs", "round", "floor", "ceil",
                                  "C", "=", "MC", "M+", "M-"};
        int row = 0, col = 0;
        for (const QString &btText : sciButtons) {
            QPushButton *btn = new QPushButton(btText, this);
            btn->setStyleSheet("font-size: 16px;");
            connect(btn, &QPushButton::clicked, this, &ScientificCalculatorWidget::onSciButtonClicked);
            grid->addWidget(btn, row, col);
            col++;
            if(col == 5) { col = 0; row++; }
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
    }
    void setFunction(const QString &func) {
        functionStr = func;
        update(); // إعادة رسم
    }
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
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
        // رقمياً: نعرف x من -10 إلى 10
        double xmin = -10.0, xmax = 10.0;
        int nPoints = w; // نقطة لكل بكسل تقريباً
        for (int i = 0; i < nPoints; i++) {
            double x = xmin + (xmax - xmin) * i / (nPoints - 1);
            double y = 0.0;
            try {
                // استبدال x بالدالة: نعتمد على محلل التعبير
                std::string exprStr = functionStr.toStdString();
                // لاستعمال قيمة x نستخدم صيغة: استبدال كل ظهور للحرف 'x'
                size_t pos = 0;
                while ((pos = exprStr.find("x", pos)) != std::string::npos) {
                    exprStr.replace(pos, 1, "(" + std::to_string(x) + ")");
                    pos += std::to_string(x).size() + 2;
                }
                y = evaluateExpression(exprStr);
            } catch (...) {
                y = 0.0;
            }
            // تحويل الإحداثيات إلى النظام الرسومي
            double screenX = (i * 1.0);
            double screenY = h/2 - y * (h/20.0); // مقياس بسيط
            points.push_back(QPointF(screenX, screenY));
        }
        painter.setPen(Qt::blue);
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
        mainLayout->addWidget(functionEdit);
        
        plotButton = new QPushButton("ارسم الدالة", this);
        plotButton->setStyleSheet("font-size: 16px;");
        connect(plotButton, &QPushButton::clicked, this, &GraphingCalculatorWidget::onPlotClicked);
        mainLayout->addWidget(plotButton);
        
        graphWidget = new GraphPlotWidget(this);
        graphWidget->setMinimumHeight(300);
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
            fa = evaluateExpression(expr.toStdString() + "(with x=" + std::to_string(a) + ")");
            fb = evaluateExpression(expr.toStdString() + "(with x=" + std::to_string(b) + ")");
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
                fm = evaluateExpression(expr.toStdString() + "(with x=" + std::to_string(m) + ")");
            } catch (...) {
                resultEdit->setPlainText("خطأ في تقييم f(m).");
                return;
            }
            if (fabs(fm) < tol)
                break;
            if(fa * fm < 0) {
                b = m; fb = fm;
            } else {
                a = m; fa = fm;
            }
        }
        resultEdit->setPlainText("الجذر التقريبي: " + QString::number(m));
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
        mainLayout->addWidget(equationEdit);
        
        QHBoxLayout *rangeLayout = new QHBoxLayout();
        lowerEdit = new QLineEdit(this);
        lowerEdit->setPlaceholderText("a");
        lowerEdit->setStyleSheet("font-size: 16px;");
        upperEdit = new QLineEdit(this);
        upperEdit->setPlaceholderText("b");
        upperEdit->setStyleSheet("font-size: 16px;");
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
            std::string s_expr = expr.toStdString();
            f_plus = evaluateExpression(s_expr + "(with x=" + std::to_string(x+h) + ")");
            f_minus = evaluateExpression(s_expr + "(with x=" + std::to_string(x-h) + ")");
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
                double fx = evaluateExpression(s_expr + "(with x=" + std::to_string(x) + ")");
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
            std::string s_expr = expr.toStdString();
            f1 = evaluateExpression(s_expr + "(with x=" + std::to_string(x0+h) + ")");
            f2 = evaluateExpression(s_expr + "(with x=" + std::to_string(x0-h) + ")");
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
        mainLayout->addWidget(calcEdit);
        
        QHBoxLayout *row1 = new QHBoxLayout();
        QLabel *pointLabel = new QLabel("لحساب المشتقة عند x =", this);
        pointLabel->setStyleSheet("font-size: 16px;");
        pointEdit = new QLineEdit(this);
        pointEdit->setStyleSheet("font-size: 16px;");
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
        QLabel *toLabel = new QLabel("إلى", this);
        toLabel->setStyleSheet("font-size: 16px;");
        upperIntEdit = new QLineEdit(this);
        upperIntEdit->setStyleSheet("font-size: 16px;");
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
        QStringList parts = numsStr.split(",", QString::SkipEmptyParts);
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
        QString resultText;
        resultText += "المتوسط الحسابي: " + QString::number(mean) + "\n";
        resultText += "الوسيط: " + QString::number(median) + "\n";
        resultText += "التباين: " + QString::number(var) + "\n";
        resultText += "الانحراف المعياري: " + QString::number(stdev) + "\n";
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
            } else if(op == "طرح") {
                res = subtractMatrix(A, B);
            } else if(op == "ضرب") {
                res = multiplyMatrix(A, B);
            } else if(op == "محدد") {
                double det = determinant(A);
                resultEdit->setPlainText("المحدد: " + QString::number(det));
                return;
            } else if(op == "معكوس") {
                res = inverseMatrix(A);
            }
            resultEdit->setPlainText(matrixToString(res));
        } catch (std::exception &ex) {
            resultEdit->setPlainText("حدث خطأ أثناء عملية المصفوفة.");
        }
    }
private:
    QTextEdit *matrixAEdit;
    QTextEdit *matrixBEdit;
    QComboBox *opCombo;
    QTextEdit *resultEdit;
    bool parseMatrix(const QString &text, std::vector<std::vector<double>> &mat) {
        mat.clear();
        QStringList rows = text.split("\n", QString::SkipEmptyParts);
        for(const QString &row : rows) {
            QStringList numbers = row.split(QRegExp("[,\\s]+"), QString::SkipEmptyParts);
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
        size_t m = A.size(), n = B[0].size();
        std::vector<std::vector<double>> C(m, std::vector<double>(n, 0));
        for (size_t i = 0; i < m; i++)
            for (size_t j = 0; j < n; j++)
                for (size_t k = 0; k < A[0].size(); k++)
                    C[i][j] += A[i][k] * B[k][j];
        return C;
    }
    double determinant(const std::vector<std::vector<double>> &M) {
        // حساب المحدد باستخدام طريقة الاستيفاء (للأبعاد الصغيرة)
        size_t n = M.size();
        if(n == 0) return 0;
        if(n == 1) return M[0][0];
        if(n == 2) return M[0][0]*
