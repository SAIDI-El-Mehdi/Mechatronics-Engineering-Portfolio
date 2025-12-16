from datetime import datetime
import os
import tkinter as tk
from tkinter import ttk, messagebox
from tkinter.simpledialog import askstring
import requests
from PIL import Image, ImageTk
import matplotlib.pyplot as plt
import json
import matplotlib.dates as mdates
class WeatherApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Météo")
        # Charger l'image d'arrière-plan et la redimensionner pour qu'elle remplisse la fenêtre
        self.bg_image = Image.open("icons/me3.jpg")  # Remplacez par le chemin de votre image
        self.bg_image = self.bg_image.resize((self.root.winfo_screenwidth(), self.root.winfo_screenheight()))  # Redimensionner l'image
        self.bg_photo = ImageTk.PhotoImage(self.bg_image)

        # Créer un label pour afficher l'image d'arrière-plan
        self.background_label = tk.Label(self.root, image=self.bg_photo)
        self.background_label.place(relwidth=1, relheight=1)
        
        # Champ de saisie pour la ville (using tk.Label for more customization)
        self.city_label = tk.Label(root, text="Entrez le nom de la ville:", font=('Arial', 16, 'bold'), bg=root.cget("bg"), fg='black')
        self.city_label.pack(pady=10)
        
        self.city_entry = ttk.Entry(root,font=('Arial', 18), width=20)
        self.city_entry.pack(pady=10)
        
        # Create a style for the ttk.Button
        style = ttk.Style()
        style.configure("TButton",
                        background="blue",   # Set the background color
                        foreground="black",  # Set the text color
                        font=('Arial', 16, 'bold'),
                        borderwidth=1,
                        relief="solid")

        # Create the button with the custom style
        self.search_button = ttk.Button(root, text="Rechercher", command=self.get_city, style="TButton")
        self.search_button.pack(pady=10)

        # Change the cursor to a pointer when hovering over the button
        self.search_button.bind("<Enter>", lambda e: self.search_button.config(cursor="hand2"))
        self.search_button.bind("<Leave>", lambda e: self.search_button.config(cursor=""))
        
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
        
        
        for i in range(7):
            day_frame = ttk.Frame(self.forecast_frame)
            day_frame.pack(side='left', expand=True)
            
            # Label for day name (e.g., "Mon", "Tue")
            day_name_label = ttk.Label(day_frame, text="--", font=('Arial', 10, 'bold'))
            day_name_label.pack(pady=5)
            self.forecast_labels.append(day_name_label)
    
            # Label for weather icon (e.g., sun, cloud, rain icon)
            weather_icon_label = ttk.Label(day_frame)
            weather_icon_label.pack(pady=5)
            self.water_icons.append(weather_icon_label)
    
            # Label for temperature (e.g., "15°C / 7°C")
            temp_label = ttk.Label(day_frame, text="--°C / --°C", font=('Arial', 12))
            temp_label.pack(pady=5)
    
            # Store the temperature labels so they can be updated later
            self.forecast_labels.append(temp_label)
        # Button to visualize weather history
        self.visualize_button = ttk.Button(root, text="Visualiser l'historique", command=self.visualize_weather_history, style="TButton")
        self.visualize_button.pack(pady=10)
        
        # Button to save weather data
        self.save_button = ttk.Button(root, text="Sauvegarder les données", command=lambda: self.save_weather_data(self.weather_data["daily"], self.ville), style="TButton")
        self.save_button.pack(pady=10)
        self.ville="oued zem"
        self.get_weather(self.ville)

    def get_weather(self,city_name):
        city = city_name
        if not self.check_internet_connection():
            self.show_custom_popup("Erreur,Aucune connexion Internet. Veuillez vérifier votre connexion.")
            return
        if not city:
            self.show_custom_popup("Erreur,Veuillez entrer un nom de ville.")
            return
        
        # Utilisation de l'API Open-Meteo pour récupérer les prévisions sur 7 jours
        geocoding_url = f"https://geocoding-api.open-meteo.com/v1/search?name={city}&count=1&language=fr"
    
        try:
            # Obtenez les coordonnées de la ville
            geocoding_response = requests.get(geocoding_url)
            geocoding_response.raise_for_status()  # Raise error for bad HTTP status
            geocoding_data = geocoding_response.json()
            
            if 'results' not in geocoding_data or not geocoding_data['results']:  # Check if city is found
                self.show_custom_popup("Erreur,Ville non trouvée.")
                return
            
            latitude = geocoding_data['results'][0]['latitude']
            longitude = geocoding_data['results'][0]['longitude']
            
            # Récupérer les prévisions de la semaine avec les coordonnées
            weather_url = f"https://api.open-meteo.com/v1/forecast?latitude={latitude}&longitude={longitude}&daily=temperature_2m_max,temperature_2m_min,weathercode&current=relative_humidity_2m,wind_speed_10m&timezone=GMT"
            weather_response = requests.get(weather_url, timeout=5)  # Timeout set to 5 seconds
            weather_response.raise_for_status()
            
            # Check if the response contains valid data
            if 'daily' not in weather_response.json() or 'current' not in weather_response.json():
                self.show_custom_popup("Erreur,Données météorologiques invalides.")
                return
            
            self.weather_data = weather_response.json()
            daily_data = self.weather_data['daily']
            current_data=self.weather_data["current"]
            # Appeler les méthodes pour mettre à jour l'interface utilisateur
            self.update_current_weather(daily_data,current_data,city)
            self.update_weekly_forecast(daily_data)
    
        
        except requests.exceptions.Timeout:
            # Timeout de la requête, l'utilisateur sera informé via le message
            self.show_custom_popup("Erreur,La requête a expiré. Veuillez réessayer.")
        except requests.exceptions.RequestException as e:
            # Erreurs liées au réseau ou à l'API
            self.show_custom_popup(f"Erreur lors de la récupération des données : {e}")
        except ValueError as e:
            # Erreurs lors du traitement des données JSON
            self.show_custom_popup(f"Erreur lors du traitement des données : {e}")
    def get_city(self):
        self.ville= self.city_entry.get()
        self.get_weather(self.ville)   
    
    def update_current_weather(self, daily_data,current_data,city):
        """Mettre à jour la météo actuelle à partir des prévisions."""
        today_temp_max = daily_data['temperature_2m_max'][0]
        today_temp_min = daily_data['temperature_2m_min'][0]
        today_humidity = current_data['relative_humidity_2m']
        today_wind_speed = current_data['wind_speed_10m']
        
        self.temp_label.config(text=f"{today_temp_max}°/{today_temp_min}°")
        self.humidity_label.config(text=f"Humidité : {today_humidity}%")
        self.wind_speed_label.config(text=f"Vent : {today_wind_speed} km/h")
        self.city_name_label.config(text=city)
        self.update_current_weather_icon(daily_data['weathercode'][0])

    def update_weekly_forecast(self, daily_data):
        # Days of the week starting from today
        days_of_week = ["Lundi", "Mardi", "Mercredi", "Jeudi", "Vendredi", "Samedi", "Dimanche"]
    
        # Get the current day (today's date)
        today = datetime.now()
    
        # Calculate the current day index (0 for Monday, 1 for Tuesday, etc.)
        today_index = today.weekday()  # Monday = 0, Sunday = 6

        # Loop through the next 7 days starting from today
        for i in range(7):
            # Get the day name for the current day
            day_name = days_of_week[(today_index + i) % 7]
            
            # Get the max and min temperatures for the current day
            temp_max = daily_data['temperature_2m_max'][(today_index + i) % 7]
            temp_min = daily_data['temperature_2m_min'][(today_index + i) % 7]
            
            # Get the weather condition code for the current day
            condition_code = daily_data['weathercode'][(today_index + i) % 7]
            
            # Update the day name label
            self.forecast_labels[i * 2].config(text=day_name)  # Access day name label
            
            # Update weather icon
            self.update_weekly_weather_icon(i, condition_code)  # This will set the icon
            
            # Update temperature label
            self.forecast_labels[i * 2 + 1].config(text=f"{temp_max}°C / {temp_min}°C") # Access temperature label

    def get_weather_condition(self, weather_code):
        """Retourne une condition météo en texte en fonction du code météo."""
        conditions = {
            0: "Ensoleillé",
            1: "Partiellement nuageux",
            2: "Nuageux",
            3: "Pluie",
            4: "Pluie forte",
            5: "Neige",
            6: "Neige forte",
            7: "Brume",
            8: "Vent",
        }
        return conditions.get(weather_code, "Condition inconnue")

    def update_current_weather_icon(self, weather_code):
        """Update the weather icon with a resized PNG image based on the weather code."""
    
        # Clear any previous image or text from the label
        self.weather_icon_label.config(image='', text='')

        # Determine the path to the appropriate PNG image based on the weather code
        icon_path = ""
        if weather_code == 0:  # Sunny
            icon_path = 'icons/sun.png'
        elif weather_code in [1, 2]:  # Cloudy
            icon_path = 'icons/cloudy.png'
        elif weather_code == 3:  # Rain
            icon_path = 'icons/cloudy.png'
        
        if icon_path:
            # Open, resize, and display the PNG image
            icon = Image.open(icon_path)
            icon = icon.resize((50, 50), Image.LANCZOS)  # Adjust size as needed
            photo = ImageTk.PhotoImage(icon)
            self.weather_icon_label.config(image=photo)
            self.weather_icon_label.image = photo  # Keep a reference to avoid garbage collection

    def update_weekly_weather_icon(self, day_index, condition_code):
        """Update the daily weather icon with a PNG image based on the weather condition code."""
    
        # Determine the path to the appropriate PNG image based on the condition code
        icon_path = ""
        if condition_code in [3, 4]:  # Light or heavy rain
            icon_path = 'icons/cloudy.png'
        elif condition_code in [5, 6]:  # Light or heavy snow
            icon_path = 'icons/snow.png'
        else:
            icon_path = 'icons/sun.png'  # Default to a sunny icon
        
        if icon_path:
            # Open, resize, and display the PNG image
            icon = Image.open(icon_path)
            icon = icon.resize((30, 30), Image.LANCZOS)  # Adjust size as needed
            photo = ImageTk.PhotoImage(icon)
            self.water_icons[day_index].config(image=photo)
            self.water_icons[day_index].image = photo  # Keep a reference to avoid garbage collection
    
    def save_weather_data(self, weather_data, city):
        """Save weather data for each city in a separate JSON file."""
        try:
            # Create a list of 7 entries, one for each day, in the week
            weather_entry = {
                'city': city,
                'week': [
                    {
                        'date': weather_data['time'][i],
                        'temperature_max': weather_data['temperature_2m_max'][i],
                        'temperature_min': weather_data['temperature_2m_min'][i]
                    }
                    for i in range(7)
                ]
            }
            # Define file path for each city
            filename = f"json/{city.lower()}_weather.json"
            # Ensure the 'json' folder exists
            if not os.path.exists('json'):
                os.makedirs('json')
            # Load existing data for the city, if available
            if os.path.exists(filename):
                with open(filename, 'r') as file:
                    data = json.load(file)
            else:
                data = []
    
            # Append the new entry and save it
            data.append(weather_entry)
            with open(filename, 'w') as file:
                json.dump(data, file, indent=4)

        except Exception as e:
            messagebox.showerror("Erreur", f"Erreur de sauvegarde des données météo: {e}")

    def visualize_weather_history(self):
        """Display a list of cities to choose from and visualize the selected city's weather data."""
        try:
            # Find all JSON files for available cities
            city_files = [f for f in os.listdir("json") if f.endswith('_weather.json')]

            if not city_files:
                messagebox.showerror("Erreur", "Aucune donnée météorologique trouvée.")
                return

            # Extract city names from the file names
            city_list = [f.split('_weather.json')[0].capitalize() for f in city_files]

            # Create a new Tkinter window for selecting the city
            select_window = tk.Toplevel(self.root)
            select_window.title("Sélectionner une ville")
            select_window.geometry("400x300")  # Set a fixed size for the window
            select_window.configure(bg='#f0f4f7')  # Set a light background color

            # Add a title label with modern font and styling
            title_label = tk.Label(select_window, text="Sélectionnez une ville", font=('Arial', 18, 'bold'), bg='#f0f4f7', fg='#333')
            title_label.pack(pady=(30, 10))

            # Create a styled dropdown (ComboBox) for selecting the city
            city_var = tk.StringVar(select_window)
            city_var.set(city_list[0])  # Default value (first city in the list)

            city_menu = ttk.Combobox(select_window, textvariable=city_var, values=city_list, font=('Arial', 12), state='readonly')
            city_menu.pack(pady=20, ipadx=5, ipady=5)  # Added padding for better spacing

            # Style the ComboBox dropdown to make it more modern
            city_menu.config(width=20, height=6, background='#f5f5f5', foreground='#333')

            def on_select():
                city_choice = city_var.get()
                if not city_choice:
                    messagebox.showerror("Erreur", "Ville non valide ou non trouvée.")
                    return

                # Load data for the selected city
                filename = f"json/{city_choice.lower()}_weather.json"
                with open(filename, 'r') as file:
                    data = json.load(file)

                # Extract dates, min and max temperatures for plotting (using the week's data)
                week_data = data[-1]['week']  # Get the last week's data (if multiple weeks saved)
                dates = [entry['date'] for entry in week_data]
                temp_min = [entry['temperature_min'] for entry in week_data]  # Min temperatures
                temp_max = [entry['temperature_max'] for entry in week_data]  # Max temperatures

                # Plot with annotations
                plt.style.use('seaborn-darkgrid')
                fig, ax = plt.subplots(figsize=(10, 6), dpi=100)

                # Plot min temperatures in blue
                ax.plot(dates, temp_min, color='blue', marker='o', linewidth=2.5, alpha=0.8, label='Température min (°C)')
                # Plot max temperatures in red
                ax.plot(dates, temp_max, color='red', marker='o', linewidth=2.5, alpha=0.8, label='Température max (°C)')

                ax.set_title(f"Historique des températures pour {city_choice}", fontsize=16, color='navy', weight='bold')
                ax.set_xlabel("Date", fontsize=12, color='grey')
                ax.set_ylabel("Température (°C)", fontsize=12, color='grey')
                plt.xticks(rotation=45, ha='right', color='black')
                plt.yticks(color='black')

                # Fill area between min and max temperatures
                ax.fill_between(dates, temp_min, temp_max, color='orange', alpha=0.2)

                # Annotate points with temperature values
                for i, (min_temp, max_temp) in enumerate(zip(temp_min, temp_max)):
                    ax.annotate(f"{min_temp}°C", (dates[i], min_temp), textcoords="offset points", xytext=(0, 8), ha='center', color='black', fontsize=10, weight='bold')
                    ax.annotate(f"{max_temp}°C", (dates[i], max_temp), textcoords="offset points", xytext=(0, 8), ha='center', color='black', fontsize=10, weight='bold')
                
                # Show the legend
                ax.legend()
                
                plt.tight_layout()
                plt.show()
                #select_window.destroy()  # Close the window after selection

            # Modern button for visualizing
            select_button = tk.Button(select_window, text="Visualiser", command=on_select, font=('Arial', 12, 'bold'), bg='#4CAF50', fg='white', relief="flat", width=20, height=2)
            select_button.pack(pady=20)

            select_window.mainloop()
        except FileNotFoundError:
            messagebox.showerror("Erreur", "Données météorologiques introuvables.")
        except json.JSONDecodeError:
            messagebox.showerror("Erreur", "Erreur de lecture des données météo.")
    
    def check_internet_connection(self):
        try:
            # Try to send a request to a reliable site (e.g., Google)
            response = requests.get("http://google.com", timeout=5)
            return response.status_code == 200
        except requests.ConnectionError:
            return False
    
    def show_custom_popup(self,message):
        # Create a new top-level window for the custom popup
        popup = tk.Toplevel()
        popup.title("Information")
        
        # Set window size and background color
        popup.geometry("300x150")
        popup.config(bg="#f0f0f0")
        
        # Center the popup on the screen
        screen_width = popup.winfo_screenwidth()
        screen_height = popup.winfo_screenheight()
        popup_width = 300
        popup_height = 150
        x_position = (screen_width - popup_width) // 2
        y_position = (screen_height - popup_height) // 2
        popup.geometry(f"{popup_width}x{popup_height}+{x_position}+{y_position}")
    
        label = tk.Label(popup, text=message)
        label.pack(pady=20)

        # Add a button to close the popup
        close_button = tk.Button(popup, text="Close", font=("Arial", 10), bg="#4CAF50", fg="white", command=popup.destroy)
        close_button.pack(pady=10)

        popup.mainloop()
if __name__ == "__main__":
    root = tk.Tk()
    app = WeatherApp(root)
    root.mainloop()
