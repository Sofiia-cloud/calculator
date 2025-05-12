#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <QWidget>
#include <QMap>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QPushButton;
class QComboBox;
class QTabWidget;
class QLabel;
QT_END_NAMESPACE

class Calculator : public QWidget
{
    Q_OBJECT

public:
    Calculator(QWidget *parent = nullptr);

private slots:
    void digitClicked();
    void operatorClicked();
    void equalClicked();
    void clearClicked();
    void clearAllClicked();
    void pointClicked();
    void backspaceClicked();
    void unaryOperatorClicked();
    void changeTheme();
    void solveQuadraticEquation();
    void convertUnits();
    void updateUnitComboBoxes();

private:
    void createStandardCalculator();
    void createScientificCalculator();
    void createUnitConverter();
    void createQuadraticSolver();
    void calculate();
    double calculateTrigFunction(const QString &func, double value);
    QPushButton *createButton(const QString &text, const char *member, const QString &style = "");
    QLineEdit *converterInput;
    QLineEdit *converterResult;
    QTabWidget *tabWidget;
    QLineEdit *display;
    QComboBox *themeComboBox;
    QComboBox *unitTypeComboBox;
    QComboBox *fromUnitComboBox;
    QComboBox *toUnitComboBox;
    QLineEdit *aLineEdit;
    QLineEdit *bLineEdit;
    QLineEdit *cLineEdit;
    QLineEdit *resultLineEdit;

    bool waitingForOperand;
    double result;
    double memory;
    QString pendingOperator;
    QMap<QString, QMap<QString, double>> unitConversionFactors;
};
#endif
