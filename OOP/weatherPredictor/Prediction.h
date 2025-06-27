#ifndef PREDICTION_H
#define PREDICTION_H

#include <vector>
#include <string>
#include <functional>
#include "Candlestick.h"
#include "Common.h"

/**
 * Structure to hold prediction results with confidence metrics
 */
struct PredictionResult {
    double predictionValue;      // The predicted temperature value
    double confidenceMetric;     // Confidence/quality metric (0.0 to 1.0 or specific range)
    std::string modelName;       // Descriptive name of the prediction model
    bool isValid;               // Whether the prediction is valid
    std::string errorMessage;   // Error description if prediction failed
    std::string confidenceDescription; // Description of what the confidence metric means
    
    // Constructor for successful prediction
    PredictionResult(double prediction, double confidence, const std::string& name, 
                    const std::string& confDesc = "") 
        : predictionValue(prediction), confidenceMetric(confidence), modelName(name),
          isValid(true), errorMessage(""), confidenceDescription(confDesc) {}
    
    // Constructor for failed prediction
    PredictionResult(const std::string& error, const std::string& name)
        : predictionValue(0.0), confidenceMetric(0.0), modelName(name),
          isValid(false), errorMessage(error), confidenceDescription("") {}
    
    // Default constructor
    PredictionResult() : predictionValue(0.0), confidenceMetric(0.0), modelName("Unknown"),
                        isValid(false), errorMessage("Uninitialized"), confidenceDescription("") {}
};

/**
 * Structure to hold cross-validation results
 */
struct ValidationResult {
    double meanAbsoluteError;    // MAE from cross-validation
    double meanSquaredError;     // MSE from cross-validation  
    double maxError;            // Maximum error observed
    double minError;            // Minimum error observed
    int validPredictions;       // Number of successful predictions
    int totalAttempts;          // Total validation attempts
    bool isValid;              // Whether validation succeeded
    std::string errorMessage;   // Error description if validation failed
    
    ValidationResult() : meanAbsoluteError(0.0), meanSquaredError(0.0), maxError(0.0), 
                        minError(0.0), validPredictions(0), totalAttempts(0), 
                        isValid(false), errorMessage("Uninitialized") {}
};

/**
 * Function pointer type for prediction models
 */
using PredictionFunction = std::function<PredictionResult(const std::vector<Candlestick>&)>;

/**
 * TASK 4: Enhanced Temperature Prediction System with Confidence Metrics
 * 
 * Provides robust prediction methods with confidence metrics and cross-validation
 */
class Prediction {
public:
    // Enhanced prediction methods returning PredictionResult with confidence metrics
    
    /**
     * Linear Regression Prediction with R² Confidence Metric
     * 
     * Uses least squares method with comprehensive confidence calculation
     * Confidence metric: R² value (0.0 to 1.0, higher = better fit)
     * 
     * @param data Vector of temperature candlestick data
     * @return PredictionResult with prediction, R² confidence, and model details
     */
    static PredictionResult predictLinearWithConfidence(const std::vector<Candlestick>& data);

    /**
     * Moving Average Prediction with Stability Confidence Metric
     * 
     * Calculates moving average with stability-based confidence
     * Confidence metric: Inverse of recent volatility (0.0 to 1.0, higher = more stable)
     * 
     * @param data Vector of temperature candlestick data
     * @param windowSize Number of recent periods to average
     * @return PredictionResult with prediction, stability confidence, and model details
     */
    static PredictionResult predictMovingAverageWithConfidence(const std::vector<Candlestick>& data, 
                                                              int windowSize = 3);

    /**
     * Heuristic (Momentum) Prediction with Consistency Confidence Metric
     * 
     * Projects momentum with consistency-based confidence
     * Confidence metric: Based on trend consistency (0.0 to 1.0, higher = more consistent)
     * 
     * @param data Vector of temperature candlestick data
     * @return PredictionResult with prediction, consistency confidence, and model details
     */
    static PredictionResult predictHeuristicWithConfidence(const std::vector<Candlestick>& data);

    // Cross-validation methods for model evaluation
    
    /**
     * Validate model using leave-one-out cross-validation
     * 
     * Performs comprehensive model validation by:
     * 1. Using each data point as test case
     * 2. Training on all previous points
     * 3. Calculating prediction errors
     * 4. Computing statistical metrics
     * 
     * @param data Vector of temperature data for validation
     * @param modelFunction Function pointer to prediction model
     * @param minTrainingSize Minimum number of points needed for training
     * @return ValidationResult with MAE, MSE, and other metrics
     */
    static ValidationResult validateModel(const std::vector<Candlestick>& data,
                                        PredictionFunction modelFunction,
                                        int minTrainingSize = 2);

    /**
     * Validate all prediction models and compare performance
     * 
     * @param data Vector of temperature data
     * @return Vector of ValidationResult for each model
     */
    static std::vector<ValidationResult> validateAllModels(const std::vector<Candlestick>& data);

    /**
     * Generate rolling predictions for visualization and comparison
     * 
     * Creates predictions for each historical period using only data available
     * up to that point, enabling comparison with actual values
     * 
     * @param data Vector of temperature data
     * @param startIndex Starting index for predictions (minimum data needed)
     * @return Vector of PredictionResult for each period
     */
    static std::vector<PredictionResult> generateRollingPredictions(
        const std::vector<Candlestick>& data,
        PredictionFunction modelFunction,
        int startIndex = 2
    );

    /**
     * Create prediction comparison chart
     * 
     * Generates a chart showing actual temperatures alongside predictions
     * from all three models for visual comparison and validation
     * 
     * @param data Vector of temperature data
     * @param timeframe Data aggregation level
     * @param country Country code being analyzed
     * @param startYear Starting year of data range
     * @param endYear Ending year of data range
     */
    static void displayPredictionComparisonChart(const std::vector<Candlestick>& data,
                                               TimeFrame timeframe,
                                               const std::string& country,
                                               int startYear, int endYear);

    // Display methods for enhanced results
    
    /**
     * Display prediction results with confidence metrics
     * 
     * Shows enhanced prediction analysis including:
     * - All three models with confidence metrics
     * - Confidence interpretations
     * - Model recommendations based on confidence
     * - Cross-validation results if requested
     * 
     * @param data Vector of temperature data
     * @param timeframe Data aggregation level
     * @param country Country code being analyzed
     * @param startYear Starting year of data range
     * @param endYear Ending year of data range
     * @param showValidation Whether to include cross-validation analysis
     */
    static void displayEnhancedPredictionResults(const std::vector<Candlestick>& data,
                                                TimeFrame timeframe,
                                                const std::string& country,
                                                int startYear, int endYear,
                                                bool showValidation = false);

    /**
     * Display cross-validation results
     * 
     * @param results Vector of validation results for different models
     */
    static void displayValidationResults(const std::vector<ValidationResult>& results);

    // Legacy methods (maintained for backward compatibility)
    static double predictLinear(const std::vector<Candlestick>& data);
    static double predictMovingAverage(const std::vector<Candlestick>& data, int windowSize = 3);
    static double predictHeuristic(const std::vector<Candlestick>& data);
    static void displayPredictionResults(const std::vector<Candlestick>& data,
                                       TimeFrame timeframe,
                                       const std::string& country,
                                       int startYear, int endYear);

    // Helper methods
    static double calculateLinearRSquared(const std::vector<Candlestick>& data);
    static std::string determineNextPeriod(const std::vector<Candlestick>& data,
                                         TimeFrame timeframe, int endYear);

private:
    // Internal calculation methods
    
    /**
     * Calculate R² coefficient of determination for linear regression
     * 
     * R² = 1 - (Residual Sum of Squares / Total Sum of Squares)
     * Range: 0.0 to 1.0 (higher = better fit)
     * 
     * @param data Vector of temperature data
     * @param slope Linear regression slope
     * @param intercept Linear regression intercept
     * @return R² value
     */
    static double calculateRSquaredDetailed(const std::vector<Candlestick>& data, 
                                          double slope, double intercept);

    /**
     * Calculate stability confidence for moving average
     * Based on inverse of recent volatility
     * 
     * @param data Vector of temperature data
     * @param windowSize Window size for calculation
     * @return Stability confidence (0.0 to 1.0)
     */
    static double calculateStabilityConfidence(const std::vector<Candlestick>& data, 
                                             int windowSize);

    /**
     * Calculate consistency confidence for heuristic model
     * Based on trend consistency in recent periods
     * 
     * @param data Vector of temperature data
     * @return Consistency confidence (0.0 to 1.0)
     */
    static double calculateConsistencyConfidence(const std::vector<Candlestick>& data);

    /**
     * Helper method to calculate mean of values
     */
    static double calculateMean(const std::vector<double>& values);

    /**
     * Helper method to calculate standard deviation
     */
    static double calculateStandardDeviation(const std::vector<double>& values, double mean);

    /**
     * Get month name from number (1-12)
     */
    static std::string getMonthName(int month);
};

#endif