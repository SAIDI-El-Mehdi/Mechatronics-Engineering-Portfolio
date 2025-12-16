import tkinter as tk
from tkinter import ttk, messagebox
import requests
from PIL import Image, ImageTk
import json
import matplotlib.pyplot as plt

class WeatherApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Météo")
        self.root.configure(bg='cyan')
        
        # Champ de saisie pour la ville
        self.city_label = ttk.Label(root, text="Entrez le nom de la ville:")
        self.city_label.pack(pady=10)
        
        self.city_entry = ttk.Entry(root, font=('Arial', 14))
        self.city_entry.pack(pady=10)
        
        self.search_button = ttk.Button(root, text="Rechercher", command=self.get_city)
        self.search_button.pack(pady=10)
        
        # Nouveaux boutons
        self.plot_button = ttk.Button(root, text="Afficher la variation", command=self.plot_temperature)
        self.plot_button.pack(pady=5)

        self.save_button = ttk.Button(root, text="Sauvegarder JSON", command=self.save_json)
        self.save_button.pack(pady=5)

        self.load_button = ttk.Button(root, text="Charger JSON", command=self.load_json)
        self.load_button.pack(pady=5)
        
        # Frame principale pour la météo
        self.main_frame = ttk.Frame(root)
        self.main_frame.pack(padx=20, pady=10, fill='both', expand=True)
        
        # Section du temps actuel
        self.top_frame = ttk.Frame(self.main_frame)
        self.top_frame.pack(fill='x', pady=(0, 20))
        
        self.temp_label = ttk.Label(self.top_frame, text="--", font=('Arial', 40))
        self.temp_label.pack(side='left')
        
        self.degree_label = ttk.Label(self.top_frame, text="°C", font=('Arial', 20))
        self.degree_label.pack(side='left', pady=(0, 20))
        
        self.weather_icon_label = ttk.Label(self.top_frame)
        self.weather_icon_label.pack(side='left', padx=(10, 20))  # Espacement entre l'icône et la température
        
        self.details_frame = ttk.Frame(self.top_frame)
        self.details_frame.pack(side='left', padx=20)
        
        self.humidity_label = ttk.Label(self.details_frame, text="Humidité : --%")
        self.humidity_label.pack(anchor='w')
        
        self.wind_speed_label = ttk.Label(self.details_frame, text="Vent : -- km/h")
        self.wind_speed_label.pack(anchor='w')

        # City name under temperature
        self.city_name_label = ttk.Label(self.top_frame, text="--", font=('Arial', 16))
        self.city_name_label.pack(side='left', padx=(10, 20))

        # Section des prévisions de la semaine
        self.forecast_frame = ttk.Frame(self.main_frame)
        self.forecast_frame.pack(fill='x', pady=20)
        
        # Initialisation pour les prévisions
        self.forecast_labels = []
        self.water_icons = []  # Store the water icons for each day
        self.weather_data = None  # Stocke les données météo actuelles
        
        for i in range(7):
            day_frame = ttk.Frame(self.forecast_frame)
            day_frame.pack(side='left', expand=True)
            
            day_name_label = ttk.Label(day_frame, text="--", font=('Arial', 10, 'bold'))
            day_name_label.pack(pady=5)
            self.forecast_labels.append(day_name_label)
    
            weather_icon_label = ttk.Label(day_frame)
            weather_icon_label.pack(pady=5)
            self.water_icons.append(weather_icon_label)
    
            temp_label = ttk.Label(day_frame, text="--°C / --°C", font=('Arial', 12))
            temp_label.pack(pady=5)
    
            self.forecast_labels.append(temp_label)
        
        self.get_weather("oued zem")
    
    def get_weather(self, city_name):
        city = city_name
        if not city:
            messagebox.showerror("Erreur", "Veuillez entrer un nom de ville.")
            return
        
        # Utilisation de l'API Open-Meteo pour récupérer les prévisions sur 7 jours
        geocoding_url = f"https://geocoding-api.open-meteo.com/v1/search?name={city}&count=1&language=fr"
    
        try:
            geocoding_response = requests.get(geocoding_url)
            geocoding_response.raise_for_status()
            geocoding_data = geocoding_response.json()
            
            if not geocoding_data['results']:
                messagebox.showerror("Erreur", "Ville non trouvée.")
                return
            
            latitude = geocoding_data['results'][0]['latitude']
            longitude = geocoding_data['results'][0]['longitude']
            
            weather_url = f"https://api.open-meteo.com/v1/forecast?latitude={latitude}&longitude={longitude}&daily=temperature_2m_max,temperature_2m_min,weathercode,relative_humidity_2m_max,windspeed_10m_max&timezone=Europe%2FParis"
            weather_response = requests.get(weather_url, timeout=5)
            weather_response.raise_for_status()
            
            if 'daily' not in weather_response.json():
                messagebox.showerror("Erreur", "Données météorologiques invalides.")
                return
            
            self.weather_data = weather_response.json()['daily']
            
            self.update_current_weather(self.weather_data, city)
            self.update_weekly_forecast(self.weather_data)
        
        except requests.exceptions.Timeout:
            messagebox.showerror("Erreur", "La requête a expiré. Veuillez réessayer.")
        except requests.exceptions.RequestException as e:
            messagebox.showerror("Erreur", f"Erreur lors de la récupération des données : {e}")
    
    def get_city(self):
        ville = self.city_entry.get()
        self.get_weather(ville)
    
    def update_current_weather(self, daily_data, city):
        today_temp_max = daily_data['temperature_2m_max'][0]
        today_temp_min = daily_data['temperature_2m_min'][0]
        today_humidity = daily_data['relative_humidity_2m_max'][0]
        today_wind_speed = daily_data['windspeed_10m_max'][0]
        
        self.temp_label.config(text=f"{today_temp_max}°/{today_temp_min}°")
        self.humidity_label.config(text=f"Humidité : {today_humidity}%")
        self.wind_speed_label.config(text=f"Vent : {today_wind_speed} km/h")
        self.city_name_label.config(text=city)

    def update_weekly_forecast(self, daily_data):
        days_of_week = ["Lundi", "Mardi", "Mercredi", "Jeudi", "Vendredi", "Samedi", "Dimanche"]
    
        for i in range(7):
            day_name = days_of_week[i]
            temp_max = daily_data['temperature_2m_max'][i]
            temp_min = daily_data['temperature_2m_min'][i]
            
            self.forecast_labels[i * 2].config(text=day_name)
            self.forecast_labels[i * 2 + 1].config(text=f"{temp_max}°C / {temp_min}°C")
    
    def plot_temperature(self):
        if not self.weather_data:
            messagebox.showerror("Erreur", "Aucune donnée disponible pour l'instant.")
            return
        
        days = ["Lundi", "Mardi", "Mercredi", "Jeudi", "Vendredi", "Samedi", "Dimanche"]
        temperatures = self.weather_data['temperature_2m_max']
        
        plt.figure(figsize=(10, 5))
        plt.plot(days, temperatures, marker='o', label="Température max")
        plt.title("Variation de température sur 7 jours")
        plt.xlabel("Jour")
        plt.ylabel("Température (°C)")
        plt.grid(True)
        plt.legend()
        plt.show()
    
    def save_json(self):
        if not self.weather_data:
            messagebox.showerror("Erreur", "Aucune donnée à sauvegarder.")
            return
        
        try:
            with open("weather_data.json", "w") as file:
                json.dump(self.weather_data, file, indent=4)
            messagebox.showinfo("Succès", "Données sauvegardées dans 'weather_data.json'.")
        except Exception as e:
            messagebox.showerror("Erreur", f"Échec de la sauvegarde : {e}")
    
    def load_json(self):
        try:
            with open("weather_data.json", "r") as file:
                self.weather_data = json.load(file)
            messagebox.showinfo("Succès", "Données chargées depuis 'weather_data.json'.")
            self.update_weekly_forecast(self.weather_data)
        except Exception as e:
            messagebox.showerror("Erreur", f"Échec du chargement : {e}")

if __name__ == "__main__":
    root = tk.Tk()
    app = WeatherApp(root)
    root.mainloop()
