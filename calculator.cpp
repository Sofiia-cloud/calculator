#include "calculator.h"
#include <QGridLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QTabWidget>
#include <QLabel>
#include <QGroupBox>
#include <QApplication>
#include <QMessageBox>
#include <cmath>
#include <QFormLayout>

Calculator::Calculator(QWidget *parent)
    : QWidget(parent), waitingForOperand(true), result(0.0), memory(0.0), pendingOperator("")
{
    // преобразование единиц
    unitConversionFactors["Длина"] = {
        {"Метры", 1.0},
        {"Километры", 1000.0},
        {"Дециметры", 0.1},
        {"Сантиметры", 0.01},
        {"Миллиметры", 0.001},
        {"Мили", 1609.34},
        {"Футы", 0.3048},
        {"Дюймы", 0.0254}
    };

    unitConversionFactors["Масса"] = {
        {"Килограммы", 1.0},
        {"Граммы", 0.001},
        {"Центнеры", 100.0},
        {"Тонны", 1000.0},
        {"Фунты", 0.453592},
        {"Унции", 0.0283495}
    };

    unitConversionFactors["Температура"] = {
        {"Цельсий", 1.0},
        {"Фаренгейт", 1.0},
        {"Кельвин", 1.0}
    };

    // настройка дисплея
    display = new QLineEdit("0");
    display->setReadOnly(true);
    display->setAlignment(Qt::AlignRight);
    display->setMaxLength(15);

    QFont font = display->font();
    font.setPointSize(font.pointSize() + 8);
    display->setFont(font);

    // создание вкладок
    tabWidget = new QTabWidget(this);

    createScientificCalculator();
    createUnitConverter();
    createQuadraticSolver();

    // добавление тем
    themeComboBox = new QComboBox();
    themeComboBox->addItem("Светлая", "light");
    themeComboBox->addItem("Темная", "dark");
    themeComboBox->addItem("Синяя", "blue");
    connect(themeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &Calculator::changeTheme);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(themeComboBox);
    mainLayout->addWidget(tabWidget);

    setLayout(mainLayout);
    setWindowTitle("Расширенный калькулятор");
    resize(500, 600);

    changeTheme();
}



void Calculator::createScientificCalculator() // научный калькулятор
{
    QWidget *scientificCalc = new QWidget();
    QVBoxLayout *scientificLayout = new QVBoxLayout(scientificCalc);

    scientificLayout->addWidget(display);

    QGridLayout *buttonsLayout = new QGridLayout();


    for (int i = 0; i < 10; ++i) {
        QPushButton *button = createButton(QString::number(i), SLOT(digitClicked()));
        int row = ((9 - i) / 3) + 2;
        int column;
        if (i%3==0){
            column = 2;
        } else if (i%3==2){
            column = 1;
        } else{
            column = 0;
        }

        if (i == 0) {
            row = 5;
            column = 0;
            buttonsLayout->addWidget(button, row, column, 1, 2);
            column += 2;
        } else {
            buttonsLayout->addWidget(button, row, column);
        }
    }

    QPushButton *pointButton = createButton(".", SLOT(pointClicked()));
    buttonsLayout->addWidget(pointButton, 5, 2);

    const QString operators[] = { "+", "-", "x", ":" };
    for (int i = 0; i < 4; ++i) {
        QPushButton *button = createButton(operators[i], SLOT(operatorClicked()));
        buttonsLayout->addWidget(button, i + 1, 3);
    }

    QPushButton *backspaceButton = createButton("⌫", SLOT(backspaceClicked()));
    buttonsLayout->addWidget(backspaceButton, 1, 0);

    QPushButton *clearButton = createButton("C", SLOT(clearClicked()));
    buttonsLayout->addWidget(clearButton, 1, 1);

    QPushButton *clearAllButton = createButton("CE", SLOT(clearAllClicked()));
    buttonsLayout->addWidget(clearAllButton, 1, 2);

    QPushButton *equalButton = createButton("=", SLOT(equalClicked()));
    buttonsLayout->addWidget(equalButton, 5, 3);


    const QString scientificButtons[][2] = {
        {"sin", "sin"}, {"cos", "cos"}, {"tg", "tan"},{"ctg", "ctg"},
        {"arcsin", "asin"}, {"arccos", "acos"}, {"arctg", "atan"},{"arcсtg", "aсtan"},
        {"√", "sqrt"}, {"π", "pi"}
    };

    for (int i = 0; i < 10; ++i) {
        QPushButton *button = createButton(scientificButtons[i][0], SLOT(unaryOperatorClicked()));
        if (i<5){
            buttonsLayout->addWidget(button, i + 1, 4);
        } else {
            buttonsLayout->addWidget(button, i - 4, 5);
        }
    }


    scientificLayout->addLayout(buttonsLayout);
    tabWidget->addTab(scientificCalc, "Научный калькулятор");
}

void Calculator::createUnitConverter() // конвертер величин
{
    QWidget *converterWidget = new QWidget();
    QVBoxLayout *converterLayout = new QVBoxLayout(converterWidget);


    unitTypeComboBox = new QComboBox();
    for (const QString &unitType : unitConversionFactors.keys()) {
        unitTypeComboBox->addItem(unitType);
    }
    connect(unitTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &Calculator::updateUnitComboBoxes);

    QHBoxLayout *typeLayout = new QHBoxLayout();
    typeLayout->addWidget(new QLabel("Тип:"));
    typeLayout->addWidget(unitTypeComboBox);
    converterLayout->addLayout(typeLayout);


    fromUnitComboBox = new QComboBox();
    toUnitComboBox = new QComboBox();
    updateUnitComboBoxes();

    QHBoxLayout *fromLayout = new QHBoxLayout();
    fromLayout->addWidget(new QLabel("Из:"));
    fromLayout->addWidget(fromUnitComboBox);

    QHBoxLayout *toLayout = new QHBoxLayout();
    toLayout->addWidget(new QLabel("В:"));
    toLayout->addWidget(toUnitComboBox);

    converterLayout->addLayout(fromLayout);
    converterLayout->addLayout(toLayout);


    converterInput = new QLineEdit("0");
    converterInput->setValidator(new QDoubleValidator(this));
    QHBoxLayout *valueLayout = new QHBoxLayout();
    valueLayout->addWidget(new QLabel("Значение:"));
    valueLayout->addWidget(converterInput);
    converterLayout->addLayout(valueLayout);


    converterResult = new QLineEdit();
    converterResult->setReadOnly(true);
    QHBoxLayout *resultLayout = new QHBoxLayout();
    resultLayout->addWidget(new QLabel("Результат:"));
    resultLayout->addWidget(converterResult);
    converterLayout->addLayout(resultLayout);


    QPushButton *convertButton = createButton("Конвертировать", SLOT(convertUnits()));
    converterLayout->addWidget(convertButton);

    tabWidget->addTab(converterWidget, "Конвертер величин");
}

void Calculator::createQuadraticSolver() { // решение квадратных уравнений
    QWidget *quadraticWidget = new QWidget();
    QGridLayout *mainLayout = new QGridLayout(quadraticWidget);


    aLineEdit = new QLineEdit("0");
    bLineEdit = new QLineEdit("0");
    cLineEdit = new QLineEdit("0");

    mainLayout->addWidget(new QLabel("Коэффициент a:"), 0, 0);
    mainLayout->addWidget(aLineEdit, 0, 1);
    mainLayout->addWidget(new QLabel("Коэффициент b:"), 1, 0);
    mainLayout->addWidget(bLineEdit, 1, 1);
    mainLayout->addWidget(new QLabel("Коэффициент c:"), 2, 0);
    mainLayout->addWidget(cLineEdit, 2, 1);


    QPushButton *solveButton = new QPushButton("Решить");
    connect(solveButton, &QPushButton::clicked,
            this, &Calculator::solveQuadraticEquation); // Подключение сигнала к слоту
    mainLayout->addWidget(solveButton, 3, 0, 1, 2);

    resultLineEdit = new QLineEdit();
    resultLineEdit->setReadOnly(true);
    mainLayout->addWidget(new QLabel("Результат:"), 4, 0);
    mainLayout->addWidget(resultLineEdit, 4, 1);

    tabWidget->addTab(quadraticWidget, "Квадратные уравнения");
}

QPushButton *Calculator::createButton(const QString &text, const char *member, const QString &style)
{
    QPushButton *button = new QPushButton(text);
    button->setMinimumSize(40, 40);
    if (!style.isEmpty()) {
        button->setStyleSheet(style);
    }
    connect(button, SIGNAL(clicked()), this, member);
    return button;
}

void Calculator::digitClicked()
{
    QPushButton *clickedButton = qobject_cast<QPushButton *>(sender());
    if (!clickedButton) return;

    QString digitValue = clickedButton->text();

    if (display->text() == "0" && digitValue == "0")
        return;

    if (waitingForOperand) {
        display->clear();
        waitingForOperand = false;
    }

    display->setText(display->text() + digitValue);
}

void Calculator::operatorClicked()
{
    QPushButton *clickedButton = qobject_cast<QPushButton *>(sender());
    if (!clickedButton) return;

    QString clickedOperator = clickedButton->text();
    double operand = display->text().toDouble();

    if (!pendingOperator.isEmpty()) {
        calculate();
        display->setText(QString::number(result));
    } else {
        result = operand;
    }

    pendingOperator = clickedOperator;
    waitingForOperand = true;
}

void Calculator::equalClicked()
{
    if (!pendingOperator.isEmpty()) {
        calculate();
        pendingOperator.clear();
        waitingForOperand = true;
    }
}

void Calculator::pointClicked()
{
    if (waitingForOperand)
        display->setText("0");

    if (!display->text().contains('.'))
        display->setText(display->text() + ".");

    waitingForOperand = false;
}

void Calculator::backspaceClicked()
{
    if (waitingForOperand)
        return;

    QString text = display->text();
    text.chop(1);

    if (text.isEmpty()) {
        text = "0";
        waitingForOperand = true;
    }

    display->setText(text);
}

void Calculator::clearClicked()
{
    if (waitingForOperand)
        return;

    display->setText("0");
    waitingForOperand = true;
}

void Calculator::clearAllClicked()
{
    display->setText("0");
    waitingForOperand = true;
    result = 0.0;
    pendingOperator.clear();
}

void Calculator::calculate()
{
    double operand = display->text().toDouble();

    if (pendingOperator == "+") {
        result += operand;
    } else if (pendingOperator == "-") {
        result -= operand;
    } else if (pendingOperator == "x") {
        result *= operand;
    } else if (pendingOperator == ":") {
        if (operand == 0.0) {
            display->setText("Ошибка");
            waitingForOperand = true;
            result = 0.0;
            pendingOperator.clear();
            return;
        }
        result /= operand;
    }

    QString formatted = QString::number(result, 'f', 10);
    while (formatted.contains('.') && (formatted.endsWith('0') || formatted.endsWith('.'))) {
        formatted.chop(1);
        if (formatted.endsWith('.')) {
            formatted.chop(1);
            break;
        }
    }
    display->setText(formatted);
}

void Calculator::unaryOperatorClicked()
{
    QPushButton *clickedButton = qobject_cast<QPushButton *>(sender());
    if (!clickedButton) return;

    QString clickedOperator = clickedButton->text();
    double operand = display->text().toDouble();
    double result = 0.0;

    if (clickedOperator == "√") {
        if (operand < 0.0) {
            display->setText("Ошибка");
            return;
        }
        result = sqrt(operand);
    } else if (clickedOperator == "π") {
        result = M_PI;
    } else {
        result = calculateTrigFunction(clickedOperator, operand);
    }

    QString formatted = QString::number(result, 'f', 10);
    while (formatted.contains('.') && (formatted.endsWith('0') || formatted.endsWith('.'))) {
        formatted.chop(1);
        if (formatted.endsWith('.')) {
            formatted.chop(1);
            break;
        }
    }
    display->setText(formatted);
    waitingForOperand = true;
}

double Calculator::calculateTrigFunction(const QString &func, double value)
{
    if (func == "sin") return sin(value);
    if (func == "cos") return cos(value);
    if (func == "tan") return tan(value);
    if (func == "asin") return asin(value);
    if (func == "acos") return acos(value);
    if (func == "atan") return atan(value);
    if (func == "ctg") return 1/tan(value);
    if (func == "actan") return 3.14/2.0 - atan(value);
    return 0.0;
}


void Calculator::changeTheme() // настройка тем
{
    QString theme = themeComboBox->currentData().toString();
    QString styleSheet;

    if (theme == "light") {
        styleSheet = "QWidget { background-color: #f0f0f0; color: black; }"
                     "QLineEdit { background-color: white; color: black; border: 1px solid gray; }"
                     "QPushButton { background-color: #e0e0e0; border: 1px solid gray; }"
                     "QComboBox { background-color: white; color: black; }"
                     "QTabWidget { background-color: #f0f0f0; }"
                     "QTabBar::tab { background: #e0e0e0; color: black; }";
    }
    else if (theme == "dark") {
        styleSheet = "QWidget { background-color: #303030; color: white; }"
                     "QLineEdit { background-color: #202020; color: white; border: 1px solid #505050; }"
                     "QPushButton { background-color: #404040; color: white; border: 1px solid #505050; }"
                     "QComboBox { background-color: #404040; color: white; }"
                     "QTabWidget { background-color: #303030; }"
                     "QTabBar::tab { background: #404040; color: white; }";
    }
    else if (theme == "blue") {
        styleSheet = "QWidget { background-color: #e6f3ff; color: black; }"
                     "QLineEdit { background-color: white; color: black; border: 1px solid #99c2ff; }"
                     "QPushButton { background-color: #cce0ff; border: 1px solid #99c2ff; }"
                     "QComboBox { background-color: white; color: black; }"
                     "QTabWidget { background-color: #e6f3ff; }"
                     "QTabBar::tab { background: #cce0ff; color: black; }";
    }

    qApp->setStyleSheet(styleSheet);
}

void Calculator::solveQuadraticEquation()
{
    bool ok1, ok2, ok3;
    double a = aLineEdit->text().toDouble(&ok1);
    double b = bLineEdit->text().toDouble(&ok2);
    double c = cLineEdit->text().toDouble(&ok3);

    if (!ok1 || !ok2 || !ok3) {
        resultLineEdit->setText("Ошибка ввода");
        return;
    }

    if (a == 0) {
        resultLineEdit->setText("Не квадратное уравнение");
        return;
    }

    double discriminant = b*b - 4*a*c;

    if (discriminant > 0) {
        double x1 = (-b + sqrt(discriminant)) / (2*a);
        double x2 = (-b - sqrt(discriminant)) / (2*a);
        resultLineEdit->setText(QString("x₁ = %1, x₂ = %2").arg(x1, 0, 'g', 6).arg(x2, 0, 'g', 6));
    } else if (discriminant == 0) {
        double x = -b / (2*a);
        resultLineEdit->setText(QString("x = %1").arg(x, 0, 'g', 6));
    } else {
        double realPart = -b / (2*a);
        double imaginaryPart = sqrt(-discriminant) / (2*a);
        resultLineEdit->setText(QString("x₁ = %1 + %2i, x₂ = %1 - %2i")
                                    .arg(realPart, 0, 'g', 6)
                                    .arg(imaginaryPart, 0, 'g', 6));
    }
}

void Calculator::convertUnits()
{
    QString unitType = unitTypeComboBox->currentText();
    QString fromUnit = fromUnitComboBox->currentText();
    QString toUnit = toUnitComboBox->currentText();

    bool ok;
    double value = converterInput->text().toDouble(&ok);

    if (!ok) {
        converterResult->setText("Ошибка ввода");
        return;
    }

    double result = 0.0;

    if (unitType == "Температура") {
        if (fromUnit == "Цельсий" && toUnit == "Фаренгейт") {
            result = value * 9/5 + 32;
        } else if (fromUnit == "Фаренгейт" && toUnit == "Цельсий") {
            result = (value - 32) * 5/9;
        } else if (fromUnit == "Цельсий" && toUnit == "Кельвин") {
            result = value + 273.15;
        } else if (fromUnit == "Кельвин" && toUnit == "Цельсий") {
            result = value - 273.15;
        } else if (fromUnit == "Фаренгейт" && toUnit == "Кельвин") {
            result = (value - 32) * 5/9 + 273.15;
        } else if (fromUnit == "Кельвин" && toUnit == "Фаренгейт") {
            result = (value - 273.15) * 9/5 + 32;
        } else {
            result = value;
        }
    } else {
        double fromFactor = unitConversionFactors[unitType][fromUnit];
        double toFactor = unitConversionFactors[unitType][toUnit];
        result = value * fromFactor / toFactor;
    }

    converterResult->setText(QString::number(result));
}

void Calculator::updateUnitComboBoxes()
{
    QString unitType = unitTypeComboBox->currentText();
    QString currentFrom = fromUnitComboBox->currentText();
    QString currentTo = toUnitComboBox->currentText();

    fromUnitComboBox->clear();
    toUnitComboBox->clear();

    for (const QString &unit : unitConversionFactors[unitType].keys()) {
        fromUnitComboBox->addItem(unit);
        toUnitComboBox->addItem(unit);
    }

    int fromIndex = fromUnitComboBox->findText(currentFrom);
    if (fromIndex >= 0) fromUnitComboBox->setCurrentIndex(fromIndex);

    int toIndex = toUnitComboBox->findText(currentTo);
    if (toIndex >= 0) toUnitComboBox->setCurrentIndex(toIndex);
}
