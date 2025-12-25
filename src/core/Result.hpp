#pragma once

#include "core/Error.hpp"

#include <optional>
#include <variant>

namespace blueplayer::core {

/**
 * @brief Type résultat pour la gestion d'erreurs fonctionnelle
 *
 * Encapsule soit une valeur de succès de type T, soit une erreur.
 * Permet une gestion d'erreurs explicite sans exceptions.
 *
 * @tparam T Le type de la valeur en cas de succès
 */
template <typename T>
class Result {
public:
  /**
   * @brief Crée un résultat de succès
   * @param value La valeur de succès
   * @return Un Result contenant la valeur
   */
  [[nodiscard]] static Result success(T value) { return Result(std::move(value)); }

  /**
   * @brief Crée un résultat d'échec
   * @param error L'erreur
   * @return Un Result contenant l'erreur
   */
  [[nodiscard]] static Result failure(Error error) { return Result(std::move(error)); }

  /**
   * @brief Vérifie si le résultat est un succès
   * @return true si succès
   */
  [[nodiscard]] bool isSuccess() const { return std::holds_alternative<T>(m_data); }

  /**
   * @brief Vérifie si le résultat est un échec
   * @return true si échec
   */
  [[nodiscard]] bool isFailure() const { return !isSuccess(); }

  /**
   * @brief Obtient la valeur (const ref)
   * @return La valeur
   * @throws std::bad_variant_access si le résultat est un échec
   */
  [[nodiscard]] const T& value() const& { return std::get<T>(m_data); }

  /**
   * @brief Obtient la valeur (rvalue ref)
   * @return La valeur déplacée
   * @throws std::bad_variant_access si le résultat est un échec
   */
  [[nodiscard]] T&& value() && { return std::get<T>(std::move(m_data)); }

  /**
   * @brief Obtient la valeur ou une valeur par défaut
   * @param defaultValue La valeur par défaut
   * @return La valeur ou la valeur par défaut
   */
  [[nodiscard]] T valueOr(T defaultValue) const {
    if (isSuccess()) {
      return std::get<T>(m_data);
    }
    return defaultValue;
  }

  /**
   * @brief Obtient l'erreur
   * @return L'erreur
   * @throws std::bad_variant_access si le résultat est un succès
   */
  [[nodiscard]] const Error& error() const { return std::get<Error>(m_data); }

  /**
   * @brief Applique une fonction à la valeur si succès
   * @tparam F Type de la fonction
   * @param f La fonction à appliquer
   * @return Un nouveau Result avec la valeur transformée ou l'erreur originale
   */
  template <typename F>
  [[nodiscard]] auto map(F&& f) const -> Result<decltype(f(std::declval<T>()))> {
    using U = decltype(f(std::declval<T>()));
    if (isSuccess()) {
      return Result<U>::success(f(std::get<T>(m_data)));
    }
    return Result<U>::failure(std::get<Error>(m_data));
  }

  /**
   * @brief Applique une fonction retournant un Result si succès
   * @tparam F Type de la fonction
   * @param f La fonction à appliquer
   * @return Le Result retourné par f ou l'erreur originale
   */
  template <typename F>
  [[nodiscard]] auto flatMap(F&& f) const -> decltype(f(std::declval<T>())) {
    using ResultType = decltype(f(std::declval<T>()));
    if (isSuccess()) {
      return f(std::get<T>(m_data));
    }
    return ResultType::failure(std::get<Error>(m_data));
  }

private:
  explicit Result(T value) : m_data(std::move(value)) {}
  explicit Result(Error error) : m_data(std::move(error)) {}

  std::variant<T, Error> m_data;
};

/**
 * @brief Spécialisation de Result pour void
 *
 * Utilisé quand une opération peut réussir sans valeur ou échouer avec une erreur.
 */
template <>
class Result<void> {
public:
  /**
   * @brief Crée un résultat de succès
   * @return Un Result de succès
   */
  [[nodiscard]] static Result success() { return Result(std::nullopt); }

  /**
   * @brief Crée un résultat d'échec
   * @param error L'erreur
   * @return Un Result contenant l'erreur
   */
  [[nodiscard]] static Result failure(Error error) { return Result(std::move(error)); }

  /**
   * @brief Vérifie si le résultat est un succès
   * @return true si succès
   */
  [[nodiscard]] bool isSuccess() const { return !m_error.has_value(); }

  /**
   * @brief Vérifie si le résultat est un échec
   * @return true si échec
   */
  [[nodiscard]] bool isFailure() const { return m_error.has_value(); }

  /**
   * @brief Obtient l'erreur
   * @return L'erreur
   * @throws std::bad_optional_access si le résultat est un succès
   */
  [[nodiscard]] const Error& error() const { return *m_error; }

private:
  explicit Result(std::nullopt_t) : m_error(std::nullopt) {}
  explicit Result(Error error) : m_error(std::move(error)) {}

  std::optional<Error> m_error;
};

}  // namespace blueplayer::core
