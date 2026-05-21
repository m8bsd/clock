package main

// Created by: Abdulmogeeb AlHumaid - abdulmogeeb@gmail.com
// Go port: converted from the original PHP script.
// Licence: nothing, just use it and enjoy it.

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io"
	"log"
	"net/http"
	"os"
	"time"
)

const doBaseURL = "https://api.digitalocean.com/v2"

// ---------------------------------------------------------------------------
// Configuration — set via environment variables or edit the constants below.
// ---------------------------------------------------------------------------

func token() string {
	if t := os.Getenv("DO_TOKEN"); t != "" {
		return t
	}
	return "TOKEN HERE"
}

func tagName() string {
	if t := os.Getenv("DO_TAG"); t != "" {
		return t
	}
	return "TAG NAME HERE"
}

// ---------------------------------------------------------------------------
// API types
// ---------------------------------------------------------------------------

type Droplet struct {
	ID   int    `json:"id"`
	Name string `json:"name"`
}

type DropletsResponse struct {
	Droplets []Droplet `json:"droplets"`
}

type Snapshot struct {
	ID        string `json:"id"`
	Name      string `json:"name"`
	CreatedAt string `json:"created_at"`
}

type SnapshotsResponse struct {
	Snapshots []Snapshot `json:"snapshots"`
}

// ---------------------------------------------------------------------------
// HTTP helpers
// ---------------------------------------------------------------------------

func newRequest(method, url string, body io.Reader) *http.Request {
	req, err := http.NewRequest(method, url, body)
	if err != nil {
		log.Fatalf("failed to create request: %v", err)
	}
	req.Header.Set("Authorization", "Bearer "+token())
	req.Header.Set("Content-Type", "application/json")
	return req
}

func doRequest(req *http.Request) []byte {
	client := &http.Client{Timeout: 30 * time.Second}
	resp, err := client.Do(req)
	if err != nil {
		log.Fatalf("HTTP request failed: %v", err)
	}
	defer resp.Body.Close()
	data, err := io.ReadAll(resp.Body)
	if err != nil {
		log.Fatalf("failed to read response body: %v", err)
	}
	return data
}

// ---------------------------------------------------------------------------
// DigitalOcean API calls
// ---------------------------------------------------------------------------

// listDroplets returns all droplets that carry the configured tag.
func listDroplets() []Droplet {
	url := fmt.Sprintf("%s/droplets?tag_name=%s", doBaseURL, tagName())
	req := newRequest(http.MethodGet, url, nil)
	data := doRequest(req)

	var result DropletsResponse
	if err := json.Unmarshal(data, &result); err != nil {
		log.Fatalf("failed to parse droplets response: %v", err)
	}
	return result.Droplets
}

// listSnapshots returns all droplet snapshots.
func listSnapshots() []Snapshot {
	url := fmt.Sprintf("%s/snapshots?resource_type=droplet", doBaseURL)
	req := newRequest(http.MethodGet, url, nil)
	data := doRequest(req)

	var result SnapshotsResponse
	if err := json.Unmarshal(data, &result); err != nil {
		log.Fatalf("failed to parse snapshots response: %v", err)
	}
	return result.Snapshots
}

// snap triggers a snapshot action for the given droplet ID.
func snap(dropletID int) {
	url := fmt.Sprintf("%s/droplets/%d/actions", doBaseURL, dropletID)
	payload, _ := json.Marshal(map[string]string{"type": "snapshot"})
	req := newRequest(http.MethodPost, url, bytes.NewReader(payload))
	doRequest(req)
}

// deleteSnap removes a snapshot by its ID.
func deleteSnap(snapID string) {
	url := fmt.Sprintf("%s/snapshots/%s", doBaseURL, snapID)
	req := newRequest(http.MethodDelete, url, nil)
	doRequest(req)
}

// ---------------------------------------------------------------------------
// Main logic
// ---------------------------------------------------------------------------

func main() {
	snapshots := listSnapshots()
	droplets := listDroplets()

	// Loop through snapshots; delete any that:
	//   - were NOT taken on a Friday, OR
	//   - were taken on a Friday but are 7+ days old.
	for _, s := range snapshots {
		createdAt, err := time.Parse(time.RFC3339, s.CreatedAt)
		if err != nil {
			log.Printf("could not parse date for snapshot %s (%s): %v", s.ID, s.CreatedAt, err)
			continue
		}

		isFriday := createdAt.Weekday() == time.Friday
		age := int(time.Since(createdAt).Hours() / 24)
		shouldDelete := !isFriday || (isFriday && age >= 7)

		if shouldDelete {
			fmt.Printf(
				"Snapshot found:\n  name : %s\n  id   : %s\n  date : %s\nNow deleting it...\n%s\n",
				s.Name, s.ID, s.CreatedAt,
				"##########################",
			)
			deleteSnap(s.ID)
		}
	}

	// Snapshot every tagged droplet.
	for _, d := range droplets {
		fmt.Printf("\nDroplet: %s (id=%d) is being backed up now\n", d.Name, d.ID)
		snap(d.ID)
	}
}
